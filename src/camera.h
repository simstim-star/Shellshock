#pragma once
#include <DirectXMath.h>
#include <windows.h>

class Camera {
  public:
	Camera();
	DirectX::XMMATRIX GetViewMatrix() const;
	void HandleMouse(HWND hwnd);
	void HandleZoom(short delta);
	
	inline float GetAngle() { return _angle; }
	inline void SetTarget(DirectX::FXMVECTOR target) { _target = target; }
	inline void SetAspectRatio(float aspectRatio) { _aspectRatio = aspectRatio; }

	DirectX::XMMATRIX GetProjectionMatrix() const;

  private:
	DirectX::XMVECTOR _target;
	DirectX::XMVECTOR _forward;
	DirectX::XMVECTOR _up;

	float _speed;
	float _zoom;
	float _minZoom;
	float _maxZoom;
	float _angle;
	float _moveSensitivity;
	float _zoomSensitivity;
	float _aspectRatio;
	int _edgeSize;

	POINT _lastMousePos;
};