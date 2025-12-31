#include "camera.h"
#include <algorithm>

using namespace DirectX;

Camera::Camera()
	: _camPos{XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f)}, _camUp{XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)}, _zoom{10.0f}, _minZoom{5.0f},
	  _maxZoom{15.0f}, _angle{XM_PIDIV4}, _speed{0.15f}, _lastMousePos{0, 0}
{
	XMVECTOR baseForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMMATRIX rotPitch = XMMatrixRotationX(_angle);
	XMMATRIX rotYaw = XMMatrixRotationY(XM_PIDIV4);
	XMMATRIX combinedRot = rotPitch * rotYaw;
	_camForward = XMVector3TransformNormal(baseForward, combinedRot);
}

XMMATRIX Camera::LookAt()
{
	XMVECTOR eyePos = _camPos - (_camForward * _zoom);
	return XMMatrixLookAtLH(eyePos, _camPos, _camUp);
}

void Camera::HandleMouse(HWND hwnd)
{
	if (GetFocus() != hwnd)
		return;

	RECT rect;
	GetClientRect(hwnd, &rect);
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(hwnd, &p);

	if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) {
		float dx = static_cast<float>(p.x - _lastMousePos.x);
		float sensitivity = 0.005f;
		XMMATRIX rotY = XMMatrixRotationY(dx * sensitivity);
		_camForward = XMVector3TransformNormal(_camForward, rotY);
		_camForward = XMVector3Normalize(_camForward);
	} else {
		const int edgeSize = 30;
		const int width = rect.right - rect.left;
		const int height = rect.bottom - rect.top;

		XMVECTOR flatForward = XMVector3Normalize(XMVectorSet(XMVectorGetX(_camForward), 0.0f, XMVectorGetZ(_camForward), 0.0f));
		XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMVECTOR flatRight = XMVector3Cross(worldUp, flatForward);

		if (p.x < edgeSize) {
			_camPos -= flatRight * _speed;
		} else if (p.x > width - edgeSize) {
			_camPos += flatRight * _speed;
		}

		if (p.y < edgeSize) {
			_camPos += flatForward * _speed;
		} else if (p.y > height - edgeSize) {
			_camPos -= flatForward * _speed;
		}
	}

	_lastMousePos = p;
}

void Camera::HandleZoom(short wheelDelta)
{
	const float zoomSensitivity = 2.0f;
	_zoom -= (static_cast<float>(wheelDelta) / WHEEL_DELTA) * zoomSensitivity;
	_zoom = std::clamp(_zoom, _minZoom, _maxZoom);
}