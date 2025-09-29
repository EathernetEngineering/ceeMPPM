/*
 * CeeHealth
 * Copyright (C) 2025 Chloe Eather
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <cee/mppm/files.h>

#include <gtest/gtest.h>

static const char *const filename = "/tmp/mppmFileTest.txt";
// Base 64 encoded original version of this file, makes for a good test case
// for i/o as it's a few k
static const char *const testFileData = "LyoKICogQ2VlSGVhbHRoCiAqIENvcHlyaWdod"
	"CAoQykgMjAyNSBDaGxvZSBFYXRoZXIKICoKICogVGhpcyBwcm9ncmFtIGlzIGZyZWUgc29mdH"
	"dhcmU6IHlvdSBjYW4gcmVkaXN0cmlidXRlIGl0IGFuZC9vciBtb2RpZnkgaXQgdW5kZXIgdGh"
	"lCiAqIHRlcm1zIG9mIHRoZSBHTlUgR2VuZXJhbCBQdWJsaWMgTGljZW5zZSBhcyBwdWJsaXNo"
	"ZWQgYnkgdGhlIEZyZWUgU29mdHdhcmUKICogRm91bmRhdGlvbiwgZWl0aGVyIHZlcnNpb24gM"
	"yBvZiB0aGUgTGljZW5zZSwgb3IgKGF0IHlvdXIgb3B0aW9uKSBhbnkgbGF0ZXIKICogdmVyc2"
	"lvbi4KICoKICogVGhpcyBwcm9ncmFtIGlzIGRpc3RyaWJ1dGVkIGluIHRoZSBob3BlIHRoYXQ"
	"gaXQgd2lsbCBiZSB1c2VmdWwsIGJ1dCBXSVRIT1VUIEFOWQogKiBXQVJSQU5UWTsgd2l0aG91"
	"dCBldmVuIHRoZSBpbXBsaWVkIHdhcnJhbnR5IG9mIE1FUkNIQU5UQUJJTElUWSBvciBGSVROR"
	"VNTCiAqIEZPUiBBIFBBUlRJQ1VMQVIgUFVSUE9TRS4gU2VlIHRoZSBHTlUgR2VuZXJhbCBQdW"
	"JsaWMgTGljZW5zZSBmb3IgbW9yZQogKiBkZXRhaWxzLgogKgogKiBZb3Ugc2hvdWxkIGhhdmU"
	"gcmVjZWl2ZWQgYSBjb3B5IG9mIHRoZSBHTlUgR2VuZXJhbCBQdWJsaWMgTGljZW5zZSBhbG9u"
	"ZwogKiB3aXRoIHRoaXMgcHJvZ3JhbS4gSWYgbm90LCBzZWUgPGh0dHBzOi8vd3d3LmdudS5vc"
	"mcvbGljZW5zZXMvPi4KICovCgojaW5jbHVkZSA8Y2VlL21wcG0vZmlsZXMuaD4KCiNpbmNsdW"
	"RlIDxndGVzdC9ndGVzdC5oPgoKc3RhdGljIGNvbnN0IGNoYXIgKmNvbnN0IGZpbGVuYW1lID0"
	"gIi90bXAvbXBwbUZpbGVUZXN0LnR4dCI7Ci8vIEJhc2UgNjQgZW5jb2RlZCBvcmlnaW5hbCB2"
	"ZXJzaW9uIG9mIHRoaXMgZmlsZSwgbWFrZXMgZm9yIGEgZ29vZCB0ZXN0IGNhc2UgZm9yIGkvb"
	"wpzdGF0aWMgY29uc3QgY2hhciAqY29uc3QgdGVzdEZpbGVEYXRhID0gIiI7CgpjbGFzcyBtcH"
	"BtRmlsZSA6IHB1YmxpYyB0ZXN0aW5nOjpUZXN0IHsKcHJvdGVjdGVkOgoJbXBwbUZpbGUoKSB"
	"7IGNlZTo6TG9nOjpJbml0KCk7IH0KCX5tcHBtRmlsZSgpIHsKCQlpZiAoc3RkOjpmaWxlc3lz"
	"dGVtOjpleGlzdHMoZmlsZW5hbWUpKSB7CgkJCXN0ZDo6ZmlsZXN5c3RlbTo6cmVtb3ZlKGZpb"
	"GVuYW1lKTsKCQl9CgkJY2VlOjpMb2c6OlNodXRkb3duKCk7Cgl9Cn07CgpURVNUX0YobXBwbU"
	"ZpbGUsIE9wZW4pCnsKCS8vIENyZWF0ZSBhIGZpbGUKCXsKCQljZWU6OmZpbGVzOjpGaWxlIGZ"
	"pbGUoZmlsZW5hbWUsIGNlZTo6ZmlsZXM6OkZpbGVNb2RlOjpJbk91dCB8IGNlZTo6ZmlsZXM6"
	"OkZpbGVNb2RlOjpDcmVhdGUpOwoJfQoJQVNTRVJUX1RSVUUoc3RkOjpmaWxlc3lzdGVtOjple"
	"GlzdHMoZmlsZW5hbWUpKTsKCXN0ZDo6ZmlsZXN5c3RlbTo6cmVtb3ZlKGZpbGVuYW1lKTsKfQ"
	"oKVEVTVF9GKG1wcG1GaWxlLCBSZWFkV3JpdGVUb2dldGhlcikKewoJewoJCS8vIENyZWF0ZSB"
	"hIGZpbGUKCQljZWU6OmZpbGVzOjpGaWxlIGZpbGUoZmlsZW5hbWUsCgkJCQkJCQkJICAgY2Vl"
	"OjpmaWxlczo6RmlsZU1vZGU6OkluT3V0IHwgY2VlOjpmaWxlczo6RmlsZU1vZGU6OkNyZWF0Z"
	"SwgMDY0NCk7CgoJCS8vIFdyaXRlIHRvIGl0CgkJc3RkOjpzcGFuPGNoYXI+IGZpbGVDb250ZW"
	"50RGF0YShjb25zdF9jYXN0PGNoYXIgKj4odGVzdEZpbGVEYXRhKSwgc3RkOjpzdHJsZW4odGV"
	"zdEZpbGVEYXRhKSk7CgkJLy9zdGQ6OnNwYW48Y2hhcj4gZmlsZUNvbnRlbnREYXRhKGZpbGVD"
	"b250ZW50LmJlZ2luKCksIGZpbGVDb250ZW50LmVuZCgpKTsKCQlBU1NFUlRfRVEoZmlsZS5Xc"
	"ml0ZShmaWxlQ29udGVudERhdGEpLCBmaWxlQ29udGVudERhdGEuc2l6ZSgpKTsKCgkJZmlsZS"
	"5TZXRDdXJyZW50UG9zKDAsIGNlZTo6ZmlsZXM6OlNlZWtNb2RlOjpBYnNvbHV0ZSk7OwoKCQk"
	"vLyBSZWFkIGl0IGluCgkJc3RkOjp2ZWN0b3I8Y2hhcj4gcmVzdWx0OwoJCUFTU0VSVF9FUShm"
	"aWxlLlJlYWQocmVzdWx0LCBmaWxlQ29udGVudERhdGEuc2l6ZSgpKSwgZmlsZUNvbnRlbnREY"
	"XRhLnNpemUoKSk7CgkJQVNTRVJUX1NUUkVRKHJlc3VsdC5kYXRhKCksIGZpbGVDb250ZW50RG"
	"F0YS5kYXRhKCkpOwoJfQoJc3RkOjpmaWxlc3lzdGVtOjpyZW1vdmUoZmlsZW5hbWUpOwp9Cgp"
	"URVNUX0YobXBwbUZpbGUsIFJlYWRXcml0ZVNlcGVyYXRlKQp7CgkvLyBDcmVhdGUgYSBmaWxl"
	"Cgl7CgkJY2VlOjpmaWxlczo6RmlsZSBmaWxlKGZpbGVuYW1lLCBjZWU6OmZpbGVzOjpGaWxlT"
	"W9kZTo6SW5PdXQgfCBjZWU6OmZpbGVzOjpGaWxlTW9kZTo6Q3JlYXRlLCAwNjQ0KTsKCX0KCS"
	"8vIFdyaXRlIHRvIGl0CglzdGQ6OnNwYW48Y2hhcj4gZmlsZUNvbnRlbnREYXRhKGNvbnN0X2N"
	"hc3Q8Y2hhciAqPih0ZXN0RmlsZURhdGEpLCBzdHJsZW4odGVzdEZpbGVEYXRhKSk7Cgl7CgkJ"
	"Y2VlOjpmaWxlczo6RmlsZSBmaWxlKGZpbGVuYW1lLCBjZWU6OmZpbGVzOjpGaWxlTW9kZTo6T"
	"3V0KTsKCgkJQVNTRVJUX0VRKGZpbGUuV3JpdGUoZmlsZUNvbnRlbnREYXRhKSwgZmlsZUNvbn"
	"RlbnREYXRhLnNpemUoKSk7Cgl9CglBU1NFUlRfVFJVRShzdGQ6OmZpbGVzeXN0ZW06OmV4aXN"
	"0cyhmaWxlbmFtZSkpOwoJLy8gUmVhZCBpdCBpbgoJewoJCWNlZTo6ZmlsZXM6OkZpbGUgZmls"
	"ZShmaWxlbmFtZSwgY2VlOjpmaWxlczo6RmlsZU1vZGU6OkluKTsKCQlzdGQ6OnZlY3Rvcjxja"
	"GFyPiByZXN1bHQ7CgkJQVNTRVJUX0VRKGZpbGUuUmVhZChyZXN1bHQsIGZpbGUuR2V0U2l6ZS"
	"gpKSwgZmlsZUNvbnRlbnREYXRhLnNpemUoKSk7CgkJQVNTRVJUX1NUUkVRKHJlc3VsdC5kYXR"
	"hKCksIGZpbGVDb250ZW50RGF0YS5kYXRhKCkpOwoJfQoJc3RkOjpmaWxlc3lzdGVtOjpyZW1v"
	"dmUoZmlsZW5hbWUpOwp9Cgo=";

class mppmFile : public testing::Test {
protected:
	mppmFile() { cee::Log::Init(); }
	~mppmFile() {
		if (std::filesystem::exists(filename)) {
			std::filesystem::remove(filename);
		}
		cee::Log::Shutdown();
	}
};

TEST_F(mppmFile, Open)
{
	// Create a file
	{
		cee::files::File file(filename, cee::files::FileMode::InOut | cee::files::FileMode::Create);
	}
	ASSERT_TRUE(std::filesystem::exists(filename));
	std::filesystem::remove(filename);
}

TEST_F(mppmFile, ReadWriteTogether)
{
	{
		// Create a file
		cee::files::File file(filename,
								   cee::files::FileMode::InOut | cee::files::FileMode::Create, 0644);

		// Write to it
		std::span<char> fileContentData(const_cast<char *>(testFileData), std::strlen(testFileData));
		//std::span<char> fileContentData(fileContent.begin(), fileContent.end());
		ASSERT_EQ(file.Write(fileContentData), fileContentData.size());

		file.SetCurrentPos(0, cee::files::SeekMode::Absolute);;

		// Read it in
		std::vector<char> result;
		ASSERT_EQ(file.Read(result, fileContentData.size()), fileContentData.size());
		ASSERT_EQ(result.size(), fileContentData.size());
		result.push_back('\0'); // NULL terminate to interpret as string
		ASSERT_STREQ(result.data(), fileContentData.data());
	}
	std::filesystem::remove(filename);
}

TEST_F(mppmFile, ReadWriteSeperate)
{
	// Create a file
	{
		cee::files::File file(filename, cee::files::FileMode::InOut | cee::files::FileMode::Create, 0644);
	}
	// Write to it
	std::span<char> fileContentData(const_cast<char *>(testFileData), strlen(testFileData));
	{
		cee::files::File file(filename, cee::files::FileMode::Out);

		ASSERT_EQ(file.Write(fileContentData), fileContentData.size());
	}
	ASSERT_TRUE(std::filesystem::exists(filename));
	// Read it in
	{
		cee::files::File file(filename, cee::files::FileMode::In);
		std::vector<char> result;
		ASSERT_EQ(file.Read(result, file.GetSize()), fileContentData.size());
		ASSERT_EQ(result.size(), fileContentData.size());
		result.push_back('\0'); // NULL terminate to interpret as string
		ASSERT_STREQ(result.data(), fileContentData.data());
	}
	std::filesystem::remove(filename);
}

