#include "invoker.h"

#include <windows.h>

#include <map>
#include <string>
#include <vector>
#include <variant>

class IOEOExectutor : public ApiExecutor
{
public:
    /**
     * Constructs an instance of the IOEOExectutor class.
     * @param apiName The name of the API to execute.
     * @param moduleName the name of the DLL the API is in.
     */
    IOEOExectutor(std::string apiName, std::string moduleName) : ApiExecutor(apiName, moduleName) {}
    bool Invoke() override;

    virtual ~IOEOExectutor() {
        // Clear the arguments vector
        arguments.clear();
    }

    static const int MAX_ARGS = 7;
};