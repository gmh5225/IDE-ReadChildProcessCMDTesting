
#include <windows.h>
#include <stdio.h>

#define OUTPUTBUFSIZE 66000
static char GlobalBuf[OUTPUTBUFSIZE] = {0};

char *
RunChildProcessCommand(const char *Cmd)
{
    STARTUPINFOA sinfo;
    PROCESS_INFORMATION pinfo;
    SECURITY_ATTRIBUTES sattr;
    HANDLE readfh;
    char *cbuff;

    memset(GlobalBuf, 0, sizeof(GlobalBuf));
    cbuff = (char *)GlobalBuf;

    // Initialize the STARTUPINFO struct
    ZeroMemory(&sinfo, sizeof(STARTUPINFO));
    sinfo.cb = sizeof(STARTUPINFO);
    sinfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

    // Uncomment this if you want to hide the other app's
    // DOS window while it runs
    //    sinfo.wShowWindow = SW_HIDE;
    sinfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    sinfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    sinfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    // Initialize security attributes to allow the launched app to
    // inherit the caller's STDOUT, STDIN, and STDERR
    sattr.nLength = sizeof(SECURITY_ATTRIBUTES);
    sattr.lpSecurityDescriptor = 0;
    sattr.bInheritHandle = TRUE;

    // Get a pipe from which we read
    // output from the launched app
    if (!CreatePipe(&readfh, &sinfo.hStdOutput, &sattr, 0))
    {
        // Error opening the pipe
        MessageBoxA(0, "Can't open pipe", "Error", MB_OK | MB_ICONEXCLAMATION);
        return NULL;
    }

    // Launch the app. We should return immediately (while the app is running)
    if (!CreateProcessA(0, (char *)Cmd, 0, 0, TRUE, 0, 0, 0, &sinfo, &pinfo))
    {
        MessageBoxA(0, "Can't start console app", "Error", MB_OK | MB_ICONEXCLAMATION);
        CloseHandle(sinfo.hStdInput);
        CloseHandle(readfh);
        CloseHandle(sinfo.hStdOutput);
        return 0;
    }

    // Don't need the read access to these pipes
    CloseHandle(sinfo.hStdInput);
    CloseHandle(sinfo.hStdOutput);

    // We haven't yet read app's output
    sinfo.dwFlags = 0;

    // Input and/or output still needs to be done?
    while (readfh)
    {
        // Capture more output of the app?
        // Read in upto OUTPUTBUFSIZE bytes
        if (!ReadFile(readfh, cbuff + sinfo.dwFlags, OUTPUTBUFSIZE - sinfo.dwFlags, &pinfo.dwProcessId, 0) ||
            !pinfo.dwProcessId)
        {
            // If we aborted for any reason other than that the
            // app has closed that pipe, it's an
            // error. Otherwise, the program has finished its
            // output apparently
            if (GetLastError() != ERROR_BROKEN_PIPE && pinfo.dwProcessId)
            {
                // An error reading the pipe
                MessageBoxA(0, "Can't read output of console app", "Error", MB_OK | MB_ICONEXCLAMATION);
                cbuff = NULL;
                break;
            }

            // Close the pipe
            CloseHandle(readfh);
            readfh = 0;
        }

        sinfo.dwFlags += pinfo.dwProcessId;
    }

    // Close output pipe
    if (readfh)
        CloseHandle(readfh);

    // Wait for the app to finish
    WaitForSingleObject(pinfo.hProcess, INFINITE);

    // Close process and thread handles
    CloseHandle(pinfo.hProcess);
    CloseHandle(pinfo.hThread);

    // Nul-terminate it
    if (cbuff)
        *(cbuff + sinfo.dwFlags) = 0;

    // Return the output
    return cbuff;
}

int
main(int argc, char *argv[])
{
    auto buf = RunChildProcessCommand(argv[1]);
    if (buf)
    {
        MessageBoxA(0, buf, "Success", MB_OK);
    }

    return 0;
}
