#include "../Core/App.h"
#include "../User/UserLogic.h"

void App::Render()
{
    if (FAILED(CreateDeviceResources()))
        return;

    RECT client{};
    GetClientRect(hwnd_, &client);
    const float width = static_cast<float>(client.right - client.left);
    const float height = static_cast<float>(client.bottom - client.top);

    comboPopup_ = ComboPopup{};
    clickConsumed_ = false;

    if (statusTimer_ > 0.0f)
    {
        statusTimer_ -= 0.016f;
        if (statusTimer_ <= 0.0f)
        {
            statusTimer_ = 0.0f;
            statusText_.clear();
        }
    }

    renderTarget_->BeginDraw();
    renderTarget_->SetTransform(D2D1::Matrix3x2F::Identity());
    renderTarget_->Clear(Theme::Window);

    DrawShell(width, height);
    UserLogic::OnRender(*this);

    const HRESULT result = renderTarget_->EndDraw();
    if (result == D2DERR_RECREATE_TARGET)
        DiscardDeviceResources();

    clicked_ = false;
}
