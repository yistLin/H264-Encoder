#include <iostream>

#include "log.h"

int main(int argc, const char *argv[])
{
  Log logger("Main");
  logger.log(Level::ERROR, "ker");
  return EXIT_SUCCESS;
}
