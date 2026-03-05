#include "camera.h"

using namespace DirectX;

// Most significant bit of SHORT returned by GetAsyncKeyState
constexpr auto IS_KEY_DOWN_MASK = 0x8000;

Camera::Camera()
	: _target{XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f)}, _up{XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)}, _zoom{10.0f}, _minZoom{5.0f},
	  _maxZoom{15.0f}, _angle{XM_PIDIV4}, _speed{0.15f}, _lastMousePos{0, 0}, _moveSensitivity{0.005f}, _edgeSize{30},
	  _zoomSensitivity{2.0f}
{
	XMVECTOR baseForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMMATRIX rotPitch = XMMatrixRotationX(_angle);
	XMMATRIX rotYaw = XMMatrixRotationY(XM_PIDIV4);
	XMMATRIX combinedRot = rotPitch * rotYaw;
	_forward = XMVector3TransformNormal(baseForward, combinedRot);
}

XMMATRIX Camera::GetViewMatrix() const
{
	XMVECTOR eyePos = _target - (_forward * _zoom);
	return XMMatrixLookAtLH(eyePos, _target, _up);
}

XMMATRIX Camera::GetProjectionMatrix() const
{
	return XMMatrixPerspectiveFovLH(XM_PIDIV4, _aspectRatio, 0.1f, 1000.0f);
}

DirectX::XMVECTOR Camera::GetPosition() const
{
	DirectX::XMVECTOR pos = _forward;
	DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationY(_angle);
	pos = DirectX::XMVector3TransformNormal(pos, rotation);
	pos = DirectX::XMVectorScale(pos, -_zoom);
	return DirectX::XMVectorAdd(_target, pos);
}

void Camera::HandleMouse(HWND hwnd)
{
	if (GetFocus() != hwnd)
		return;

	RECT rect;
	GetClientRect(hwnd, &rect);
	POINT cursorPosition;
	GetCursorPos(&cursorPosition);
	ScreenToClient(hwnd, &cursorPosition);

	if (GetAsyncKeyState(VK_MBUTTON) & IS_KEY_DOWN_MASK) {
		float dx = static_cast<float>(cursorPosition.x - _lastMousePos.x);
		XMMATRIX rotY = XMMatrixRotationY(dx * _moveSensitivity);
		_forward = XMVector3Normalize(XMVector3TransformNormal(_forward, rotY));
	} else {
		const int width = rect.right - rect.left;
		const int height = rect.bottom - rect.top;

		XMVECTOR normForward = XMVector3Normalize(XMVectorSet(XMVectorGetX(_forward), 0.0f, XMVectorGetZ(_forward), 0.0f));
		XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMVECTOR normRight = XMVector3Cross(worldUp, normForward);

		if (cursorPosition.x < _edgeSize) {
			_target -= normRight * _speed;
		} else if (cursorPosition.x > width - _edgeSize) {
			_target += normRight * _speed;
		}

		if (cursorPosition.y < _edgeSize) {
			_target += normForward * _speed;
		} else if (cursorPosition.y > height - _edgeSize) {
			_target -= normForward * _speed;
		}
	}

	_lastMousePos = cursorPosition;
}

void Camera::HandleZoom(short wheelDelta)
{
	_zoom -= (static_cast<float>(wheelDelta) / WHEEL_DELTA) * _zoomSensitivity;
	_zoom = std::clamp(_zoom, _minZoom, _maxZoom);
}