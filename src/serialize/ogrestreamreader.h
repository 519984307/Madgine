#pragma once


namespace Engine
{
	namespace Serialize
	{
		class OgreStreamReader : public std::basic_streambuf<char>
		{
		public:
			OgreStreamReader(Ogre::MemoryDataStreamPtr stream);

		protected:
			std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way,
			                       std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
			std::streampos seekpos(std::streampos pos,
			                       std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
		private:
			Ogre::MemoryDataStreamPtr mStream;
		};
	}
}