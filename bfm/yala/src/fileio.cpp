#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fileio.h>
#include <logging.h>

//-----------------------------------------------------------------------

bool FileExists(const char* const name)
{
  bool exists = false;
  FILE* f = fopen(name, "rb");
  if (NULL != f)
  {
    fclose(f);
    exists = true;
  }
  return exists;
}

//-----------------------------------------------------------------------

bool LoadFile(const char* const filename, std::vector<char>& content)
{
	errno = 0;
	FILE* in = fopen(filename, "rb");
	if (NULL != in)
	{
		fseek(in, 0, SEEK_END);
		const long size = ftell(in);
		content.resize(size);
		rewind(in);
		fread(content.data(), sizeof(char), content.size(), in);
		fclose(in);
	}
	else
	{
		LOG("%s - errno: %s\n", filename, strerror(errno));
	}
	return (NULL != in);
}

//-----------------------------------------------------------------------

bool ListFiles(const char* const dirname, std::vector<std::string>& files)
{
	DIR* dir = opendir(dirname);
	if (NULL != dir)
	{
		files.clear();
		for (struct dirent* ent = readdir(dir); NULL != ent; ent = readdir(dir))
		{
			if ((0 != strcmp(ent->d_name, ".")) && (0 != strcmp(ent->d_name, "..")))
			{
        char fullpath[MAX_PATH + 1];
        sprintf(fullpath, "%s\\%s", dirname, ent->d_name);
				files.push_back(fullpath);
			}
		}
		closedir(dir);
	}
	return (NULL != dir);
}
