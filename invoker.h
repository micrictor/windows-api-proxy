#ifndef INVOKER_H
#define INVOKER_H

#include <windows.h>
#include <map>
#include <string>
#include <vector>
#include <variant>


typedef std::vector<unsigned long long> argVector_t;

class ApiExecutor
{
public:
    /**
     * Constructs an instance of the ApiExecutor class.
     * @param apiName The name of the API to execute.
     * @param moduleName the name of the DLL the API is in.
     */
    ApiExecutor(std::string apiName, std::string moduleName) {
        // Get the address of the API
        HMODULE libraryHandle = GetModuleHandleA(moduleName.c_str());
        apiAddress = GetProcAddress(libraryHandle, apiName.c_str());
    }
    /**
     * Calls the API with the given arguments.
     * @param args The arguments to pass to the API call.
     * @return true if the API call succeeded, false otherwise.
     */
    template <typename... Args>
    bool Call(Args... args) {
        // Clear the arguments vector
        arguments.clear();
        // Add each argument to the vector
        (arguments.push_back((unsigned long long)args), ...);
        arguments.push_back((unsigned long long)apiAddress);
        arguments.shrink_to_fit();
        // Invoke the API call
        return Invoke();
    }
    // Invoke the API call with the given arguments.
    // This is the method each implementation must override.
    virtual bool Invoke() { return false; };

    virtual ~ApiExecutor() {
        // Clear the arguments vector
        arguments.clear();
    }
    
    // Maximum number of args supported by the method
    static const int MAX_ARGS;
protected:
    // API address
    FARPROC apiAddress;
    // Vector of arguments, accepting pretty much any type we expect.
    argVector_t arguments;
};

#endif // INVOKER_H