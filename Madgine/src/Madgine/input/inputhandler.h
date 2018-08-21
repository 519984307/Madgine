#pragma once

namespace Engine
{
	namespace Input
	{
		class MADGINE_CLIENT_EXPORT InputHandler
		{
		public:
			InputHandler();
			virtual ~InputHandler() = default;

			virtual void update() = 0;
			
			void setListener(InputListener *listener);
			InputListener *listener();

		protected:
			void injectKeyPress(const KeyEventArgs& arg);
			void injectKeyRelease(const KeyEventArgs& arg);
			void injectMousePress(const MouseEventArgs& arg);
			void injectMouseRelease(const MouseEventArgs& arg);
			void injectMouseMove(const MouseEventArgs& arg);

		private:
			InputListener* mListener;
		};
	}
}
