#pragma once

#include "Modules/resources/resourceloader.h"

namespace Engine {
	namespace Tools {

		class LayoutLoader : public Resources::ResourceLoader<LayoutLoader, tinyxml2::XMLDocument> {
			
		public:
			LayoutLoader();

		private:
			virtual std::shared_ptr<Data> loadImpl(ResourceType * res) override;
		};

	}
}