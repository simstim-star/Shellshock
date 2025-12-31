#pragma once
#include <DirectXMath.h>
#include <windows.h>

class Camera {
  public:
	Camera();
	DirectX::XMMATRIX LookAt();
	void HandleMouse(HWND hwnd);
	void HandleZoom(short delta);
	
	inline float GetAngle() { return _angle; }

  private:
	DirectX::XMVECTOR _camPos;
	DirectX::XMVECTOR _camForward;
	DirectX::XMVECTOR _camUp;

	float _speed;
	float _zoom;
	float _minZoom;
	float _maxZoom;
	float _angle;

	POINT _lastMousePos;
};