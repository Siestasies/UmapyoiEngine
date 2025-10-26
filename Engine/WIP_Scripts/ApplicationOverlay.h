#pragma once
namespace Uma_Engine
{
	class ApplicationOverlay
	{
		public:
			virtual void OnAttach() = 0;
			virtual void OnDetach() = 0;
			virtual void OnUpdate(float dt) = 0;

			virtual ~ApplicationOverlay() = default;
	};
}