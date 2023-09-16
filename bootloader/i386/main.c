


void start32()
{
  volatile char* videoMemory = (char*)0xB8000;
  videoMemory[0] = '.';
}


