#include <Windows.h>
#include <winternl.h>
#include <cstdio>

// wdm.h from the windows driver kit
#define SE_SHUTDOWN_PRIVILEGE 19UL 

// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
#define STATUS_MEMBER_IN_GROUP 0xC0000067

// https://ntdoc.m417z.com/rtladjustprivilege
using RtlAdjustPrivilege_t = NTSTATUS(NTAPI *)(
    ULONG Privilege,
    BOOLEAN Enable,
    BOOLEAN Client,
    PBOOLEAN WasEnabled
);

// https://ntdoc.m417z.com/ntraiseharderror
using NtRaiseHardError_t = NTSTATUS(NTAPI *)(
    NTSTATUS ErrorStatus,
    ULONG NumberOfParameters,
    ULONG UnicodeStringParameterMask,
    PULONG_PTR Parameters,
    ULONG ValidResponseOptions,
    PULONG Response
);

// https://ntdoc.m417z.com/harderror_response_option
namespace HARDERROR_RESPONSE_OPTION {
    enum {
        OptionAbortRetryIgnore,
        OptionOk,
        OptionOkCancel,
        OptionRetryCancel,
        OptionYesNo,
        OptionYesNoCancel,
        OptionShutdownSystem,
        OptionOkNoWait,
        OptionCancelTryContinue
    };
};

int main() {
    auto ntdll = GetModuleHandleA("ntdll.dll");
    if (not ntdll) {
        printf("Every process speaks its name\nMine waits in silence\n");
        return 1;
    }

    {
        auto RtlAdjustPrivilege = (RtlAdjustPrivilege_t)GetProcAddress(ntdll, "RtlAdjustPrivilege");
        BOOLEAN wasEnabled;
        NTSTATUS ok = RtlAdjustPrivilege(
            SE_SHUTDOWN_PRIVILEGE,
            TRUE,
            FALSE,
            &wasEnabled
        );
        if (not NT_SUCCESS(ok)) {
            printf("RtlAdjustPrivilege failed with code: 0x%X", ok);
            return 1;
        }
    }
    
    {
        auto NtRaiseHardError = (NtRaiseHardError_t)GetProcAddress(ntdll, "NtRaiseHardError");
        ULONG response;
        NTSTATUS ok = NtRaiseHardError(
            STATUS_MEMBER_IN_GROUP,
            NULL,
            NULL,
            nullptr,
            HARDERROR_RESPONSE_OPTION::OptionShutdownSystem,
            &response
        );
        if (not NT_SUCCESS(ok)) {
            printf("NtRaiseHardError failed with code: 0x%X", ok);
            return 1;
        }
    }

    return 0;
}