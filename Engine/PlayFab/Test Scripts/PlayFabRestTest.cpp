// Temporary test — delete before shipping
#include <httpClient/httpClient.h>
#include <XAsync.h>
#include <XTaskQueue.h>
#include <playfab/core/PFAuthentication.h>
#include <playfab/core/PFServiceConfig.h>
#include <playfab/services/PFTitleDataManagement.h>
#include <playfab/services/PFTitleDataManagementTypes.h>
#include <playfab/services/PFServices.h>
#include "Debugging/Debugger.hpp"
#include <string>
#include <sstream>

static const char* TITLE_ID = "FF2A5";
static const char* SECRET_KEY = "8O6GRYXHKMF1MFK677WUYUMTIC11PPHIO8EHYHXTD39HO4ARZC";
static const char* ENDPOINT = "https://FF2A5.playfabapi.com";

namespace Uma_Engine
{
    struct TestContext
    {
        XTaskQueueHandle      taskQueue{ nullptr };
        PFServiceConfigHandle serviceConfig{ nullptr };
        PFEntityHandle        entityHandle{ nullptr };
        bool                  done{ false };
    };

    static void DoSetTitleData(TestContext* ctx)
    {
        struct SetContext
        {
            TestContext* test;
            XAsyncBlock  async{};
        };
        auto* sc = new SetContext{ ctx };
        sc->async.queue = ctx->taskQueue;
        sc->async.context = sc;
        sc->async.callback = [](XAsyncBlock* ab)
            {
                auto* sc = static_cast<SetContext*>(ab->context);
                HRESULT hr = XAsyncGetStatus(ab, false);
                if (SUCCEEDED(hr))
                    Debugger::Log(WarningLevel::eInfo, "[PlayFab] ServerSetTitleData OK");
                else
                {
                    std::ostringstream oss;
                    oss << "[PlayFab] ServerSetTitleData FAILED: 0x" << std::hex << hr;
                    Debugger::Log(WarningLevel::eError, oss.str());
                }
                sc->test->done = true;
                delete sc;
            };

        PFTitleDataManagementSetTitleDataRequest req{};
        req.key = "game version";
        req.value = "HelloFromUmapyoi";

        HRESULT hr = PFTitleDataManagementServerSetTitleDataAsync(
            ctx->entityHandle,
            &req,
            &sc->async);

        if (FAILED(hr))
        {
            std::ostringstream oss;
            oss << "[PlayFab] ServerSetTitleDataAsync dispatch FAILED: 0x" << std::hex << hr;
            Debugger::Log(WarningLevel::eError, oss.str());
            ctx->done = true;
            delete sc;
        }
    }

    void TestSetTitleData()
    {
        auto* ctx = new TestContext{};

        // 0 — Initialize PlayFab Services (must be first)
        HRESULT hr = PFServicesInitialize(nullptr);
        if (FAILED(hr))
        {
            std::ostringstream oss;
            oss << "[PlayFab] PFServicesInitialize FAILED: 0x" << std::hex << hr;
            Debugger::Log(WarningLevel::eError, oss.str());
            delete ctx; return;
        }
        Debugger::Log(WarningLevel::eInfo, "[PlayFab] PFServicesInitialize OK");

        // 1 — Task queue
        hr = XTaskQueueCreate(
            XTaskQueueDispatchMode::Manual,
            XTaskQueueDispatchMode::Manual,
            &ctx->taskQueue);
        if (FAILED(hr))
        {
            std::ostringstream oss;
            oss << "[PlayFab] XTaskQueueCreate FAILED: 0x" << std::hex << hr;
            Debugger::Log(WarningLevel::eError, oss.str());
            delete ctx; return;
        }
        Debugger::Log(WarningLevel::eInfo, "[PlayFab] XTaskQueueCreate OK");

        // 2 — Service config
        hr = PFServiceConfigCreateHandle(ENDPOINT, TITLE_ID, &ctx->serviceConfig);
        if (FAILED(hr))
        {
            std::ostringstream oss;
            oss << "[PlayFab] PFServiceConfigCreateHandle FAILED: 0x" << std::hex << hr;
            Debugger::Log(WarningLevel::eError, oss.str());
            delete ctx; return;
        }
        Debugger::Log(WarningLevel::eInfo, "[PlayFab] PFServiceConfigCreateHandle OK");

        // 3 — SecretKey auth
        struct AuthContext
        {
            TestContext* test;
            XAsyncBlock  async{};
        };
        auto* ac = new AuthContext{ ctx };
        ac->async.queue = ctx->taskQueue;
        ac->async.context = ac;
        ac->async.callback = [](XAsyncBlock* ab)
            {
                auto* ac = static_cast<AuthContext*>(ab->context);
                HRESULT hr = PFAuthenticationGetEntityWithSecretKeyGetResult(ab, &ac->test->entityHandle);
                if (SUCCEEDED(hr))
                {
                    Debugger::Log(WarningLevel::eInfo, "[PlayFab] SecretKey auth OK — got entity handle");
                    DoSetTitleData(ac->test);
                }
                else
                {
                    std::ostringstream oss;
                    oss << "[PlayFab] SecretKey auth FAILED: 0x" << std::hex << hr;
                    Debugger::Log(WarningLevel::eError, oss.str());
                    ac->test->done = true;
                }
                delete ac;
            };

        PFAuthenticationGetEntityRequest req{};
        hr = PFAuthenticationGetEntityWithSecretKeyAsync(
            ctx->serviceConfig, SECRET_KEY, &req, &ac->async);
        if (FAILED(hr))
        {
            std::ostringstream oss;
            oss << "[PlayFab] GetEntityWithSecretKeyAsync dispatch FAILED: 0x" << std::hex << hr;
            Debugger::Log(WarningLevel::eError, oss.str());
            delete ac; delete ctx; return;
        }

        // 4 — Pump until done
        while (!ctx->done)
        {
            XTaskQueueDispatch(ctx->taskQueue, XTaskQueuePort::Work, 0);
            XTaskQueueDispatch(ctx->taskQueue, XTaskQueuePort::Completion, 0);
        }

        // 5 — Cleanup
        if (ctx->entityHandle) PFEntityCloseHandle(ctx->entityHandle);
        PFServiceConfigCloseHandle(ctx->serviceConfig);
        XTaskQueueCloseHandle(ctx->taskQueue);
        delete ctx;

        Debugger::Log(WarningLevel::eInfo, "[PlayFab] Test done");
    }
}