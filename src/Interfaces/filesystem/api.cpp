#include "../interfaceslib.h"
#include "api.h"

namespace Engine {
	namespace Filesystem {

		void createDirectories(const Path & p)
		{
			if (!exists(p)) {
				Path parent = p.parentPath();
				if (!parent.empty())
					createDirectories(parent);
				createDirectory(p);
			}
		}

		FileQuery listFilesRecursive(const Path & path)
		{
			return { path, true };
		}

	}
}
