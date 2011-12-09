/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     Captnoord
*/

#include "EVEServerPCH.h"
#include "authorisation/PasswordModule.h"
#include "authorisation/ShaModule.h"
#ifndef WIN32
    #include <iconv.h>
#endif

uint8 mDigest[SHA_DIGEST_SIZE];

#ifndef WIN32
    uint8 * PasswordModule::wchar_tToUtf16(uint8 * chars, int charLen)
    {
        iconv_t cd;
	size_t iconv_value;
	cd = iconv_open("UTF-16", "WCHAR_T");
	long unsigned int len;
	long unsigned int utf16len;
	char * utf16;
	uint8 * utf16start;
	len = charLen*4;
	utf16len = charLen*2;
	cd = iconv_open("UTF-16LE", "UTF-32");
	utf16 = (char *)calloc(utf16len, 1);
	utf16start = (uint8*)utf16;
	iconv_value = iconv(cd, (char**)&chars, &len, &utf16, &utf16len);
	
	return utf16start;
    }
#endif

bool PasswordModule::GeneratePassHash(std::wstring &userName, std::wstring &passWord, std::string &passWordHash)
{
    if (userName.size() > 100 || passWord.size() > 100) {
        sLog.Error("PasswordModule", "username or password is simply to long");
        return false;
    }

    std::wstring salt = userName;

    Utils::Strings::trim(salt, true, true);     // comparable to the ".strip" function in python.
    Utils::Strings::toLowerCase(salt);

    int saltLen = (int)salt.size() * 2;
    uint8 * saltChar = (uint8*)&salt[0];

    std::wstring init = passWord + salt;

    int saltInitLen = (int)init.size() * 2;
    uint8 * saltInitChar = (uint8*)&init[0];

    #ifndef WIN32
        saltChar = wchar_tToUtf16(saltChar, salt.size());
        saltInitChar = wchar_tToUtf16(saltInitChar, init.size());
    #endif

    ShaModule::SHAobject shaObj;

    ShaModule::sha_init(&shaObj);
    ShaModule::sha_update(&shaObj, saltInitChar, saltInitLen);

    int daylySaltLen = SHA_DIGEST_SIZE + saltLen;

    // allocate buffer for the hashing, this way its only allocated once.
    uint8 * uDaylySalt = (uint8 *)malloc(daylySaltLen);   // first part of the data string
    uint8 * uDaylySaltPart = &uDaylySalt[SHA_DIGEST_SIZE];      // second part of the data string

    for (int i = 0; i < 1000; i++)
    {
        ShaModule::sha_digest(&shaObj, uDaylySalt);
        memcpy(uDaylySaltPart, saltChar, saltLen);

        ShaModule::sha_init(&shaObj);
        ShaModule::sha_update(&shaObj, uDaylySalt, daylySaltLen);
    }

    free(uDaylySalt);

    ShaModule::sha_final(mDigest, &shaObj);
    
    if (passWordHash.size() != 0)
        passWordHash = "";

    passWordHash.append((char*)mDigest, SHA_DIGEST_SIZE);
    return true;
}

bool PasswordModule::GeneratePassHash(const wchar_t *userName, const wchar_t *passWord, std::string &passWordHash)
{
    if (wcsnlen(userName, 51) > 50 || wcsnlen(passWord, 51) > 50) {
        sLog.Error("PasswordModule", "username or password is simply to long");
        return false;
    }

    std::wstring salt(userName);

    Utils::Strings::trim(salt, true, true);     // comparable to the ".strip" function in python.
    Utils::Strings::toLowerCase(salt);

    int saltLen = (int)salt.size() * 2;
    uint8 * saltChar = (uint8*)&salt[0];

    std::wstring init(passWord);
    init += salt;
    
    int saltInitLen = (int)init.size() * 2;
    uint8 * saltInitChar = (uint8*)&init[0];
    
    #ifndef WIN32
        saltChar = wchar_tToUtf16(saltChar, salt.size());
        saltInitChar = wchar_tToUtf16(saltInitChar, init.size());
    #endif

    ShaModule::SHAobject shaObj;

    ShaModule::sha_init(&shaObj);
    ShaModule::sha_update(&shaObj, saltInitChar, saltInitLen);

    int daylySaltLen = SHA_DIGEST_SIZE + saltLen;

    // allocate buffer for the hashing, this way its only allocated once.
    uint8 * uDaylySalt = (uint8 *)malloc(daylySaltLen);   // first part of the data string
    uint8 * uDaylySaltPart = &uDaylySalt[SHA_DIGEST_SIZE];      // second part of the data string

    for (int i = 0; i < 1000; i++)
    {
        ShaModule::sha_digest(&shaObj, uDaylySalt);
        memcpy(uDaylySaltPart, saltChar, saltLen);

        ShaModule::sha_init(&shaObj);
        ShaModule::sha_update(&shaObj, uDaylySalt, daylySaltLen);
    }

    free(uDaylySalt);

    ShaModule::sha_final(mDigest, &shaObj);
    
    if (passWordHash.size() != 0)
        passWordHash = "";
    
    passWordHash.append((char*)mDigest, SHA_DIGEST_SIZE);

    return true;
}

std::string PasswordModule::GenerateHexString(const std::string & str)
{
    return ShaModule::Hexify(str.c_str(), str.size());
}
