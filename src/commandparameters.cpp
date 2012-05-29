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
        // TODO: error
        break;
      }

      nlen = value++ - s;

      // --name=value options
      if (strncmp("--start", s, nlen) == 0) {
        params["start"] = value;
      } else if (strncmp("--script", s, nlen) == 0) {
        params["script"] = value;
      } else if (strncmp("--command", s, nlen) == 0) {
        params["command"] = value;
      } else {
        //error
        break;
      }
    }
    return params;
}

bool CommandParameters::isCommandMode()
{
    QMap<QString, QString> params = getParams();
    return params.contains("command") && params["command"] != "";
}


bool CommandParameters::hasStart()
{
    QMap<QString, QString> params = getParams();
    return params.contains("start") && params["start"] != "";
}


bool CommandParameters::hasScript()
{
    QMap<QString, QString> params = getParams();
    return params.contains("script") && params["script"] != "";
}
