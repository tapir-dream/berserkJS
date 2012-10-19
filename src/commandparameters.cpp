#include "commandparameters.h"

CommandParameters::CommandParameters()
{
}

QMap<QString, QString> CommandParameters::getParams()
{
    int argc = qApp->argc();
    char **argv = qApp->argv();

    QMap<QString, QString> params;

    // Parse command line parameters
    for (int ax = 1; ax < argc; ++ax) {
      size_t nlen;

      const char* s = argv[ax];
      const char* value;

      value = strchr(s, '=');

      if (value == NULL) {
          if (strncmp("--help", s, 6) == 0) {
              params["help"] = "1";
          } else if (strncmp("--start", s, 7) == 0) {
              params["start"] = "1";
          } else if (strncmp("--command", s, 9) == 0) {
              params["command"] = "1";
          } else if (strncmp("--version", s, 9) == 0) {
              params["version"] = "1";
          }
          continue;
      }

      nlen = value++ - s;

      // --name=value options
      if (strncmp("--start", s, nlen) == 0) {
        params["start"] = value;
      } else if (strncmp("--script", s, nlen) == 0) {
        params["script"] = value;
      } else if (strncmp("--command", s, nlen) == 0) {
        params["command"] = value;
      } else if (strncmp("--help", s, nlen) == 0) {
        params["help"] = value;
      } else {
        //error
        continue;
      }
    }
    return params;
}

bool CommandParameters::isCommandMode()
{
    QMap<QString, QString> params = getParams();
    return params.contains("command");
}

bool CommandParameters::hasStart()
{
    QMap<QString, QString> params = getParams();
    return params.contains("start");
}


bool CommandParameters::hasScript()
{
    QMap<QString, QString> params = getParams();
    return params.contains("script") && params["script"] != "";
}


bool CommandParameters::hasHelp()
{
    QMap<QString, QString> params = getParams();
    return params.contains("help");
}

bool CommandParameters::hasVersion()
{
    QMap<QString, QString> params = getParams();
    return params.contains("version") && params["version"] != "";
}
