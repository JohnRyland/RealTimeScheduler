//
//  main.cpp
//  DescriptionHere
//
//  Created by John Ryland (jryland@xiaofrog.com) on 18/11/2017.
//  Copyright 2017 InvertedLogic. All rights reserved.
//
#include "utils.h"
#include "filesystem.h"
#include "completions.h"

#include <thread>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <cctype>
#include <vector>
#include <string>


#define hash(str) \
   ((str[0]<<24)|((str[0])?str[1]<<16:0)|((str[0]&&str[1])?str[2]<<8:0)|((str[0]&&str[1]&&str[2])?str[3]:0))

#define FOURCC(a,b,c,d) \
   (a << 24 | b << 16 | c << 8 | d)

#define CASE_(str, a,b,c,d) \
   }{ \
     case FOURCC((a),(b),(c),(d)): if (strcmp(argv[0],str) == 0)

#define CASE(str) \
   CASE_(str, str[0], (!str[0]) ? 0 : str[1], (!str[0] || !str[1]) ? 0 : str[2], (!str[0] || !str[1] || !str[2]) ? 0 : str[3])

typedef int (*command_t)(int argc, const char* argv[]);


class CommandLineWithCompletions : public FileTabCompletions
{
public:
  CommandLineWithCompletions(const std::vector<std::string>& commandList, FileSystem& fs)
    : fileSystem(fs)
    , FileTabCompletions(fs)
    , cmdCompletions(commandList)
    , inputProcessor(text, cmdCompletions, *this)
    , input(kb, inputProcessor)
  {
  }
  bool readLine(std::string& s)
  {
    showPrompt();
    bool res = input.readLine(s);
    // TODO: figure out where to show this - test 'cat' command with no arguments
    //printf("\n");
    return res;
  }
  void showPossibleCompletions(const std::vector<Completion>& completions)
  {
    // TODO: if first arg, also do tab completions from available commands
    FileTabCompletions::showPossibleCompletions(completions);
    showPrompt();
    text.display();
    fflush(stdout);
  }
private:
  void showPrompt()
  {
    printf("%s", (fileSystem.getCurrentDirectory() + "> ").c_str());
    fflush(stdout);
  }
  FileSystem& fileSystem;
  CommandTabCompletions cmdCompletions;
  CommandLineText text;
  RawKeyboard kb;
  ReadLineWithCompletions inputProcessor;
public:
  ReadLineKeyboard input;
};


class LineOutputInterface
{
public:
  virtual ~LineOutputInterface() {}
  virtual void putchar(int ch) = 0;
  // int printf();
  // int puts();
};


class Process
{
public:
  LineInputInterface*  input = nullptr;
  LineOutputInterface* output = nullptr;
  virtual void run()
  {
  }
};


class PipedInput : public LineInputInterface
{
public:
  bool isOpen() { return true; }
  bool readLine(std::string& str)
  {
    pipedSource->run();
    return true;
  }
  Process* pipedSource;
};


struct Context
{
  Context(int _argc, char** _argv, FileSystem& _fs, CommandLineWithCompletions& _c)
    : argc(_argc), argv(_argv), fs(_fs), commandLine(_c)
  {
  }

  // Need read / write to stdin/stdout here
  // Process  process;

  // Environment  ->  map(string, string)

  Context*                    parentCtx = nullptr;
  int                         argc;
  char**                      argv;
  FileSystem&                 fs;
  CommandLineWithCompletions& commandLine;
  bool                        exiting = false;
  int                         exitCode = 0;
};


int ls_cmd(Context& ctx, int argc, const char* argv[])
{
  int argCount = (argc <= 1) ? 2 : argc;
  for (int i = 1; i < argCount; i++)
  {
    std::string path = ".";
    if (argc > 1)
      path = argv[i];
    if (argc > 2)
      printf("%s%s:\n", (i!=1)?"\n":"", path.c_str());

    // Doesn't do columns or colors
    DirectoryEntrySet entries;
    if (!ctx.fs.getDirectoryListing(entries, path))
      printf("%s: %s: No such file or directory\n", ctx.fs.basename(argv[0]).c_str(), path.c_str());
    for (int i = 0; i < entries.entries.size(); i++)
      printf("%s\n", entries.entries[i].basename.c_str());
  }
  return 0;
}


int cat_cmd(Context& ctx, int argc, const char* argv[])
{
  // Handles 0 args and reading from stdin. Also handles lists of files to cat
  int argCount = (argc <= 1) ? 2 : argc;
  for (int i = 1; i < argCount; i++)
  {
    LineBasedFile file((argc > 1) ? argv[i] : NULL);
    LineInputInterface& inp = (argc > 1) ? (LineInputInterface&)file : ctx.commandLine.input;
    if (!inp.isOpen()) {
      printf("%s: %s: No such file or directory\n", ctx.fs.basename(argv[0]).c_str(), argv[i]);
      continue;
    }
    std::string line;
    while (inp.readLine(line))
    {
      printf("%s", line.c_str());
    }
  }
  return 0;
}


int head_usage(const char* err, const char* in)
{
  printf("head: %s -- %s\n", err, in);
  printf("usage: head [-n lines | -c bytes] [file ...]\n");
  return 1;
}


enum OpType
{
  Flag,
  Int,
  Char,
  Str,
  List
};


struct CmdOption
{
  bool        optional;
  bool        mutuallyExclusiveWithLast;
  char        opCh;
  const char* opStr;
  const char* opError;
  OpType      opType;
  const char* valueName;
  void*       defaultVal;
  int         minVal;
  int         maxVal;
  void*       result;
  int         resultLen;
  const char* description;
  bool        wasSet;
};


int usage_from_opts(const char* argv0, int optC, CmdOption optV[])
{
  printf("usage: %s ", argv0);
  for (int x = 0; x < optC; x++)
  {
    bool lastOpt = (x+1 == optC);
    bool mx = optV[x].mutuallyExclusiveWithLast;
    bool nextMx = (lastOpt) ? false : optV[x + 1].mutuallyExclusiveWithLast;

    if (mx)                          { printf("|%s", (optV[x].opCh != ' ') ? " " : ""); }
    if (!mx && optV[x].optional)     { printf("["); }
    if (optV[x].opCh != ' ')         { printf("-%c", optV[x].opCh); }
    if (optV[x].opType != Flag)      { printf("%s%s", (optV[x].opType != List && optV[x].valueName[0]) ? " " : "", optV[x].valueName); }
    if (optV[x].opType == List)      { printf(" ..."); }
    if (!nextMx && optV[x].optional) { printf("]"); }
    if (!lastOpt)                    { printf(" "); }
  }
  printf("\n");
  return 1;
}


int parse_args(int optC, CmdOption optV[], int argc, const char* argv[])
{
  int listIndex = -1;
  for (int x = 0; x < optC; x++)
  {
    optV[x].result = optV[x].defaultVal;
    optV[x].resultLen = 0;
    optV[x].wasSet = false;
    if (optV[x].opType == List)
    {
      if (listIndex != -1)
        printf("warning, bad command opts specification\n");
      listIndex = x;
    }
  }
  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      bool found = false;
      char optCh = argv[i][1];
      int j = 0;
      int optX = 0;
      if (optCh == '-' && argv[i][2])
      {
        for (int x = 0; x < optC; x++)
        {
          if (strcmp(&argv[i][2], optV[x].opStr) == 0)
          {
            optX = x;
            found = true;
            break;
          }
        }
      }
      else
      {
        for (int x = 0; x < optC; x++)
        {
          if (optCh == optV[x].opCh)
          {
            optX = x;
            found = true;
            if (argv[i][2] == 0)
              i++;
            else
              j = 2;
            break;
          }
        }
      }
      if (!found)
      {
        printf("%s: illegal option -- %s\n", argv[0], &argv[i][1]);
        return usage_from_opts(argv[0], optC, optV);
      }
      else
      {
        if (i == argc) {
          printf("%s: option requires an argument -- %s\n", argv[0], &argv[i-1][1]);
          return usage_from_opts(argv[0], optC, optV);
        }
        /*
        if (optV[optX].wasSet)
        {
           printf("%s: option already specified -- %s\n", argv[0], &argv[i-1][1]);
           return usage_from_opts(argv[0], optC, optV);
        }
        */
        char* endp = nullptr;
        int res = (int)strtol(&argv[i][j], &endp, 10);
        optV[optX].result = reinterpret_cast<void*>(res);
        optV[optX].wasSet = true;
        if (endp[0] || res < optV[optX].minVal || res > optV[optX].maxVal) {
          printf("%s: %s -- %s\n", argv[0], optV[optX].opError, &argv[i][j]);
          return 1;
        }
      }
    }
    else
    {
      if (listIndex != -1)
      {
        optV[listIndex].result = &argv[i];
        optV[listIndex].resultLen = argc - i;
        break;
      }
    }
  }

  bool lastSet = false;
  const char* lastSetOpt = "";
  for (int x = 0; x < optC; x++)
  {
    if (!optV[x].mutuallyExclusiveWithLast)
      lastSet = false;
    if (optV[x].wasSet)
    {
      if (lastSet)
      {
        printf("%s: can't combine %s and %s options\n", argv[0], lastSetOpt, optV[x].valueName);
        return usage_from_opts(argv[0], optC, optV);
      }
      lastSetOpt = optV[x].valueName;
      lastSet = true;
    }
  }
  return 0;
}


template <class T, std::size_t N>
static inline constexpr
std::size_t array_size(const T (&array)[N]) noexcept
{
  return N;
}


int head_cmd(Context& ctx, int argc, const char* argv[])
{
  CmdOption headOpts[] =
  {
    { true, false, 'n', "", "illegal line count",  Int, "lines", (void*)-1, 1, INT_MAX, nullptr, 0, "", false },
    { true, true,  'c', "", "illegal byte count",  Int, "bytes", (void*)-1, 1, INT_MAX, nullptr, 0, "", false },
    { true, false, ' ', "",                   "", List,  "file",   nullptr, 0,       0, nullptr, 0, "", false }
  };

  if (parse_args(array_size(headOpts), headOpts, argc, argv) == 0)
  {
    int lineCount = (int)reinterpret_cast<size_t>(headOpts[0].result);
    int byteCount = (int)reinterpret_cast<size_t>(headOpts[1].result);
    char** files = (char**)headOpts[2].result;
    int fileCount = headOpts[2].resultLen;
    bool outputFileName = fileCount > 1;

    // Default is a line count of 10 if nothing specified
    if (byteCount == -1 && lineCount == -1)
    {
      lineCount = 10;
    }

    // Handles 0 args and reading from stdin. Also handles lists of files to cat
    int argCount = (fileCount == 0) ? fileCount+1 : fileCount;
    for (int i = 0; i < argCount; i++)
    {
      LineBasedFile file((fileCount) ? files[i] : NULL);
      LineInputInterface& inp = (fileCount) ? (LineInputInterface&)file : ctx.commandLine.input;
      if (!inp.isOpen()) {
        printf("%s: %s: No such file or directory\n", ctx.fs.basename(argv[0]).c_str(), files[i]);
        continue;
      }
      if (outputFileName)
      {
        printf("==> %s <==\n", files[i]);
      }
      std::string line;
      int ln = 0;
      int cc = 0;
      while (inp.readLine(line))
      {
        cc += line.size();
        if (byteCount != -1 && cc > byteCount) {
          cc -= line.size();
          std::string rem = line.substr(0, byteCount - cc);
          printf("%s", rem.c_str());
          break;
        }
        else
          printf("%s", line.c_str());
        ln++;
        if (ln == lineCount)
          break;
      }
    }
    return 0;
  }
  return 1;
}


int cd_cmd(Context& ctx, int argc, const char* argv[])
{
  if (argc > 1) {
    if (strcmp(argv[1], "-") == 0) {
      ctx.fs.popDirectory();
    } else {
      ctx.fs.changeDirectory(argv[1]);
    }
  } else {
    ctx.fs.changeDirectory(getenv("HOME"));
  }
  return 0;
}


int exit_cmd(Context& ctx, int argc, const char* argv[])
{
  int retCode = 0;
  printf("\nExiting...\n");
  if (argc > 1)
    retCode = atoi(argv[1]);
  ctx.exiting = true;
  ctx.exitCode = retCode;
  return retCode;
}


int basename_cmd(Context& ctx, int argc, const char* argv[])
{
  if (argc < 2) {
    // printf("usage: basename string [suffix]\n       basename [-a] [-s suffix] string [...]\n");
    printf("usage: basename string\n");
    return 1;
  }
  printf("%s\n", ctx.fs.basename(argv[1]).c_str());
  return 0;
}


int dirname_cmd(Context& ctx, int argc, const char* argv[])
{
  if (argc < 2) {
    printf("usage: dirname path\n");//, ctx.fs.basename(argv[0]).c_str());
    return 1;
  }
  printf("%s\n", ctx.fs.dirname(argv[1]).c_str());
  return 0;
}


int echo_cmd(Context& ctx, int argc, const char* argv[])
{
  bool showNewLine = true;
  if (argc > 1)
  {
    int firstArg = 1;
    if (strcmp(argv[1], "-n") == 0) {
      showNewLine = false;
      firstArg = 2;
    }
    for (int i = firstArg; i < argc; i++)
    {
      if (i != firstArg)
      {
        printf(" ");
      }
      // TODO: expand variables
      printf("%s", argv[i]);
    }
  }
  if (showNewLine)
  {
    printf("\n");
  }
  return 0;
}


int true_cmd(Context& ctx, int argc, const char* argv[])
{
  return 0;
}


int false_cmd(Context& ctx, int argc, const char* argv[])
{
  return 1;
}


void process_date_format(time_t& tim, struct tm *dateTime, const char* format)
{
  const char* daysOfWeek[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
  const char* monthsOfYear[] = { "Janurary", "Feburary", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
  while (*format)
  {
    if (*format == '%')
    {
      format++;
      switch (*format)
      {
        case   0:                                                             break;
        case 'n': printf("\n");                                               break; // newline
        case 't': printf("\t");                                               break; // tab
        case 'E': printf("\b");                                               break; // backspace
        case 'O': printf("\b");                                               break; // backspace
        case 'a': printf("%.3s",  daysOfWeek[dateTime->tm_wday]);             break; // short day of week
        case 'w': printf("%i",    dateTime->tm_wday);                         break; // day of week num
        case 'd': printf("%02i",  dateTime->tm_mday);                         break; // day of month
        case 'e': printf("% 2i",  dateTime->tm_mday);                         break; // day of month
        case 'm': printf("%02i",  dateTime->tm_mon + 1);                      break; // month
        case 'b': printf("%.3s",  monthsOfYear[dateTime->tm_mon]);            break; // short month
        case 'k': printf("%02i",  dateTime->tm_hour);                         break; // 24 hour
        case 'l': printf("%i",    dateTime->tm_hour % 12);                    break; // 12 hour
        case 'I': printf("%02i",  dateTime->tm_hour % 12);                    break; // 12 hour
        case 'p': printf("%s",    (dateTime->tm_hour > 11) ? "pm" : "am");    break; // am/pm
        case 'M': printf("%02i",  dateTime->tm_min);                          break; // minute
        case 'S': printf("%02i",  dateTime->tm_sec);                          break; // second
        case 's': printf("%llu",  (unsigned long long)tim);                   break; // seconds from epoc
        case 'A': printf("%s",    daysOfWeek[dateTime->tm_wday]);             break; // day of week
        case 'B': printf("%s",    monthsOfYear[dateTime->tm_mon]);            break; // month
        case 'Y': printf("%i",    dateTime->tm_year + 1900);                  break; // year
        case 'C': printf("%i",    (dateTime->tm_year + 1900) / 100);          break; // century
        case 'y': printf("%i",    (dateTime->tm_year + 1900) % 100);          break; // short year
        case 'i': printf("%i",    dateTime->tm_yday + 1);                     break; // day from beginning of year (1 based)
        case 'Z': printf("%s",    dateTime->tm_zone);                         break; // timezone

        // Compound formats
        case 'r': process_date_format(tim, dateTime, "%l:%M:%S %p");          break;
        case 'v': process_date_format(tim, dateTime, "%e-%b-%Y");             break;
        case 'x': process_date_format(tim, dateTime, "%d/%m/%Y");             break;
        case 'D': process_date_format(tim, dateTime, "%m/%d/%y");             break;
        case 'F': process_date_format(tim, dateTime, "%Y-%m-%d");             break;
        case 'R': process_date_format(tim, dateTime, "%k:%M");                break;
        case 'T': process_date_format(tim, dateTime, "%k:%M:%S");             break;
        case 'X': process_date_format(tim, dateTime, "%k:%M:%S");             break;
        case 'c': process_date_format(tim, dateTime, "%a %e %b %k:%M:%S %Y"); break;
        
        // TODO: check there might be some subtle difference to these:
        case 'f': process_date_format(tim, dateTime, "%y");                   break;
        case 'g': process_date_format(tim, dateTime, "%y");                   break;
        case 'h': process_date_format(tim, dateTime, "%b");                   break;
        case 'j': process_date_format(tim, dateTime, "%i");                   break;
        case 'u': process_date_format(tim, dateTime, "%w");                   break;
        case 'G': process_date_format(tim, dateTime, "%Y");                   break;
        case 'H': process_date_format(tim, dateTime, "%k");                   break;
        case 'z': printf("+%i%02i", (int)dateTime->tm_gmtoff / 3600, ((int)dateTime->tm_gmtoff / 60) % 60); break; // tz offset
        case 'U': printf("%i", (((7+dateTime->tm_yday) - dateTime->tm_wday)/7));                            break; // week of year - Sunday start - starts with week 0
        case 'W': printf("%i", (((7+dateTime->tm_yday) - ((dateTime->tm_wday+6)%7))/7));                    break; // week of year - Monday start - starts with week 0
        case 'V': printf("%i", (dateTime->tm_yday - (((dateTime->tm_wday+6)%7) + 1) + 10) / 7);             break; // ISO week, TODO: incorrect around start/end of year 
        default:  printf("%c", *format);                                      break;
      }
      if (*format == 0)
        break;
    }
    else
    {
      printf("%c", *format);
    }
    format++;
  }
}


int date_cmd(Context& ctx, int argc, const char* argv[])
{
  /*
date: illegal option -- -
usage: date [-jnRu] [-d dst] [-r seconds] [-t west] [-v[+|-]val[ymwdHMS]] ... 
            [-f fmt date | [[[mm]dd]HH]MM[[cc]yy][.ss]] [+format]

     The options are as follows:

     -d dst  Set the kernel's value for daylight saving time.  If dst is non-zero, future calls to gettimeofday(2) will return a non-zero for tz_dsttime.

     -f      Use input_fmt as the format string to parse the new_date provided rather than using the default [[[mm]dd]HH]MM[[cc]yy][.ss] format.  Parsing is done using strptime(3).

     -j      Do not try to set the date.  This allows you to use the -f flag in addition to the + option to convert one date format to another.

     -n      By default, if the timed(8) daemon is running, date sets the time on all of the machines in the local group.  The -n option suppresses this behavior and causes the
             time to be set only on the current machine.

     -R      Use RFC 2822 date and time output format. This is equivalent to use ``%a, %d %b %Y %T %z'' as output_fmt while LC_TIME is set to the ``C'' locale .

     -r seconds
             Print the date and time represented by seconds, where seconds is the number of seconds since the Epoch (00:00:00 UTC, January 1, 1970; see time(3)), and can be speci-
             fied in decimal, octal, or hex.

     -r filename
             Print the date and time of the last modification of filename.

     -t minutes_west
             Set the system's value for minutes west of GMT.  minutes_west specifies the number of minutes returned in tz_minuteswest by future calls to gettimeofday(2).

     -u      Display or set the date in UTC (Coordinated Universal) time.

     -v      Adjust (i.e., take the current date and display the result of the adjustment; not actually set the date) the second, minute, hour, month day, week day, month or year
             according to val.  If val is preceded with a plus or minus sign, the date is adjusted forwards or backwards according to the remaining string, otherwise the relevant

  */
  CmdOption dateOpts[] =
  {
    { true, false, 'j', "", "", Flag,               "do not try to set the date", nullptr, 0, 0, nullptr, 0, "", false },
    { true, false, 'n', "", "", Flag, "set the date only on the current machine", nullptr, 0, 0, nullptr, 0, "", false },
    { true, false, 'R', "", "", Flag, "use RFC 2822 date and time output format", nullptr, 0, 0, nullptr, 0, "", false },
    { true, false, 'u', "", "", Flag,      "display or set the date in UTC time", nullptr, 0, 0, nullptr, 0, "", false },
    
    //{ true, false, 'd', "", "",  Str,                                      "dst", nullptr, 0, 0, nullptr, 0, "", false },
    { true, false, 'r', "", "",  Int,                                  "seconds", nullptr, 0, INT_MAX, nullptr, 0, "", false },   // if arg is int, seconds, if arg is file, modification time
    //{ true, false, 't', "", "",  Int,                                     "west", nullptr, 0, INT_MAX, nullptr, 0, "", false },

    { true, false, 'v', "", "",  Str,                        "[+|-]val[ymwdHMS]", nullptr, 0, 0, nullptr, 0, "", false },         // how much to adjust the time by
    { true, false, 'f', "", "",  Str,                                 "fmt date", nullptr, 0, 0, nullptr, 0, "", false },
    { true,  true, ' ', "", "",  Str,              "[[[mm]dd]HH]MM[[cc]yy][.ss]", nullptr, 0, 0, nullptr, 0, "", false },
    { true, false, '+', "", "", List,                               "output_fmt", nullptr, 0, 0, nullptr, 0, "", false },
  };

  if (parse_args(array_size(dateOpts), dateOpts, argc, argv) == 0)
  {
    time_t tim = time(0);
    //struct tm *dateTime = gmtime(&tim);
    struct tm *dateTime = localtime(&tim);
    const char* defaultFormatStr = "%a %e %b %Y %H:%M:%S %Z";
    const char* format = defaultFormatStr;
    // "-%a- -%b- -%c- -%d- -%e- -%g- -%h- -%j- -%k- -%l- -%m- -%n- -%p- -%r- -%s- -%t- -%u- -%v- -%w- -%x- -%y- -%z- -%A- -%B- -%C- -%D- -%E- -%F- -%G- -%H- -%I- -%J- -%K- -%L- -%M- -%N- -%O- -%P- -%Q- -%R- -%S- -%T- -%U- -%V- -%W- -%X- -%Y- -%Z-";
    process_date_format(tim, dateTime, format);
    printf("\n");
    return 1;
  }
  return 0;
}


int sleep_cmd(Context& ctx, int argc, const char* argv[])
{
  if (argc != 2) {
    printf("usage: %s seconds\n", argv[0]);
    return 1;
  }
  int secs = atoi(argv[1]);
  std::this_thread::sleep_for(std::chrono::seconds(secs));
  return 0;
}


int for_cmd(Context& ctx, int argc, const char* argv[])
{
  printf("got for\n");
  return 0;
}


int if_cmd(Context& ctx, int argc, const char* argv[])
{
  printf("got if\n");
  return 0;
}

int shell_cmd(Context& ctx, int argc, const char* argv[])
{
  printf("got shell\n");
  return 0;
}


int getopts_cmd(Context& ctx, int argc, const char* argv[])
{
/*
Johns-MacBook-Pro:mini-shell jryland$ OPTIND=0
Johns-MacBook-Pro:mini-shell jryland$ getopts S:T blah -S 123 -T ; echo $blah $OPTARG $OPTERR $OPTIND
S 123 0 3
Johns-MacBook-Pro:mini-shell jryland$ getopts S:T blah -S 123 -T ; echo $blah $OPTARG $OPTERR $OPTIND
T 0 4
Johns-MacBook-Pro:mini-shell jryland$ getopts S:T blah -S 123 -T ; echo $blah $OPTARG $OPTERR $OPTIND
? 0 4
*/
  return 0;
}


int longcommand_cmd(Context& ctx, int argc, const char* argv[])
{
  printf("got longcommand\n");
  return 0;
}


// Built-in commands
#define COMMAND_LIST \
    DO_COMMAND("ls",           ls_cmd)         \
    DO_COMMAND("cat",          cat_cmd)        \
    DO_COMMAND("cd",           cd_cmd)         \
    DO_COMMAND("date",         date_cmd)       \
    DO_COMMAND("echo",         echo_cmd)       \
    DO_COMMAND("exit",         exit_cmd)       \
    DO_COMMAND("head",         head_cmd)       \
    DO_COMMAND("sleep",        sleep_cmd)      \
    DO_COMMAND("true",         true_cmd)       \
    DO_COMMAND("false",        false_cmd)      \
    DO_COMMAND("help",         help_cmd)       \
    DO_COMMAND("for",          for_cmd)        \
    DO_COMMAND("if",           if_cmd)         \
    DO_COMMAND("basename",     basename_cmd)   \
    DO_COMMAND("dirname",      dirname_cmd)    \
    DO_COMMAND("shell",        shell_cmd)      \
    DO_COMMAND("longcommand",  longcommand_cmd)


int help_cmd(Context& ctx, int argc, const char* argv[])
{
  printf("  Command list:\n");
#define DO_COMMAND(str, cmd)      \
  printf("    %s\n", str);
  COMMAND_LIST
#undef DO_COMMAND
  return 0;
}


int exec_command(Context& ctx, int argc, const char* argv[])
{
  int ret = 0;
  if (argc > 0)
  // TODO: make the command execution logic more extensible
  switch (hash(argv[0]))
  {{
#define DO_COMMAND(str, cmd)       \
    CASE(str)                      \
    {                              \
      ret = cmd(ctx, argc, argv);  \
      break;                       \
    }
    COMMAND_LIST
#undef DO_COMMAND
    default:
    {
      printf("-%s: %s: command not found\n", ctx.fs.basename(ctx.argv[0]).c_str(), argv[0]);
    }
  }}
  return ret;
}

  
/*

TODO:
----
run processes in threads
handle Ctrl-C etc to terminate a process (key thread)
job control
pipe commands together, synchronization etc

Supported:
----------
dirname
echo
head
true
false
sleep

Partially Supported:
--------------------
basename
cat
cd
ls
date

Not-implemented:
----------------
easy:
  unalias, alias, pwd, sort, split, join, fold, tr, paste, strings, uniq, wc, who, du, df, cut, 

hard:
  vi, awk, diff, patch, sed, lex, make, mailx, ed, crontab, nm, yacc

process control:
  bg, fg, jobs, kill, nice, renice, nohup, ps, wait

file commands:
  cp, find, ln, mkdir, mkfifo, mv, rm, rmdir, chmod, chgrp, chown, touch,

Other:
  ar, asa, at, batch, bc, cal, cksum, cmp, comm, command, compress, csplit, dd, env, ex, expand, expr, fc, file,
  gencat, getconf, getopts, grep, hash, iconv, id, ipcrm, icps, locale, localedef, logger, logname, lp, man, mesg,
  more, newgrp, nl, od, pathchk, pax, pr, printf, read, sh, stty, tabs, tail, talk, tee, test, time, tput, tsort,
  tty, type, ulimit, umask, uname, uncompress, unexpand, uudecode, uuencode, write, xargs, zcat



built-in commands

if, then, else, elif, fi
case, esac
select in
while in
until
for in
do
done
function id { commands ; }
continue
break
return
time
()
{}
;
|
||
&&
&
[
[[]]
var=...
alias x='...'
eval
print

command posix/unix command list:


common env variables:

HOME
IFS
PATH
USERNAME
PWD
EDITOR
PS1
SHELL
VISUAL
TERM


echo


*/


int main(int argc, char *argv[])
{
#define DO_COMMAND(str, cmd) str,
  std::vector<std::string> cmdList = { COMMAND_LIST };
#undef DO_COMMAND

  FileSystem fs;
  CommandLineWithCompletions commandLine(cmdList, fs);
  Context ctx(argc, argv, fs, commandLine);

  char IFS[64];
  std::string rdLine;
  while (commandLine.readLine(rdLine))
  {
    // Strip trailing newline
    if (rdLine.size() && rdLine[rdLine.size()-1] == '\n')
      rdLine = rdLine.substr(0, rdLine.size()-1);

    std::vector<std::string> args = strSplit(rdLine.c_str(), ' ');
    int argc = args.size();
    std::vector<const char*> argvVec;
    for (int i = 0; i < argc; i++)
      argvVec.push_back(args[i].c_str());
    const char** argv = &argvVec[0];

    exec_command(ctx, argc, argv);
    
    if (ctx.exiting)
      return ctx.exitCode;

    /*
    printf("got: -%s-, split: ", line);
    for (int i = 0; i < args.size(); i++)
      printf(" %s, ", args[i].c_str());
    printf("\n");
    */
  }
}


