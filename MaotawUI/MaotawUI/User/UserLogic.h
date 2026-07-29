#pragma once
class App;

namespace UserLogic
{
    // Add your own behaviour in UserLogic.cpp. These hooks are already wired.
    void OnStart(App& app);
    void OnFrame(App& app);
    void OnRender(App& app);
    void OnShutdown(App& app);
}
