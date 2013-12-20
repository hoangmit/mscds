#pragma once

#include <ctime>

struct RemoteFileInfo {
	size_t filesize;
	time_t last_update;
	bool operator==(const RemoteFileInfo& other) const {
		return filesize == other.filesize && last_update == other.last_update;
	}
	bool operator!=(const RemoteFileInfo& other) const { return !(*this == other); }
};

