#include "SPIniReadWrite.h"
#include "wchar.h"

#if JUCE_WINDOWS
#include "windows.h"
#endif

static String compositeSectionString(const String& strInput);
static String compositeKeyString(const String& strKey, const String& strValue);

static void Utf8ToUnicode(const char* szUtf8, juce_wchar* szUnicode)
{
	int i = 0;
	int j = 0;
	while(szUtf8[i] != 0)
	{
		if ((szUtf8[i] & 0x80) == 0x00)//一位
		{
			szUnicode[j] = szUtf8[i];
			i += 1;
		}
		else if ((szUtf8[i] & 0xE0) == 0xC0)//两位
		{
			szUnicode[j] = ((szUtf8[i] & 0x1F) << 6) | (szUtf8[i + 1] & 0x3F);
			i += 2;
		}
		else if ((szUtf8[i] & 0xF0) == 0xE0)
		{
			szUnicode[j] = ((szUtf8[i] & 0x0F) << 12) | ((szUtf8[i + 1] & 0x3F) << 6) | 
				(szUtf8[i + 2] & 0x3F);
			i += 3;
		}
		else if ((szUtf8[i] & 0xF8) == 0xF0)
		{
			szUnicode[j] = ((szUtf8[i] & 0x07) << 18) | ((szUtf8[i + 1] & 0x3F) << 12) | 
				((szUtf8[i + 2] & 0x3F) << 6) | (szUtf8[i + 3] & 0x3F);
			i += 4;
		}
		else if ((szUtf8[i] & 0xFC) == 0xF8)
		{
			szUnicode[j] = ((szUtf8[i] & 0x03) << 24) | ((szUtf8[i + 1] & 0x3F) << 18) | 
				((szUtf8[i + 2] & 0x3F) << 12) | ((szUtf8[i + 3] & 0x3F) << 6) | (szUtf8[i + 4] & 0x3F);
			i += 5;
		}
		else if ((szUtf8[i] & 0xFE) == 0xFC)
		{
			szUnicode[j] = ((szUtf8[i] & 0x01) << 30) | ((szUtf8[i + 1] & 0x3F) << 18) |
				((szUtf8[i + 2] & 0x3F) << 18) | ((szUtf8[i + 3] & 0x3F) << 12) | 
				((szUtf8[i + 4] & 0x3F) << 6) | (szUtf8[i + 5] & 0x3F);
			i += 6;
		}
		else
		{
			break;
		}
		j += 1;
	}
	szUnicode[j] = 0;
}

CSPIniReadWrite::CSPIniReadWrite()
{

}


CSPIniReadWrite::~CSPIniReadWrite()
{
    delAllSections();
}

/*==============================================================================
Function Name:  GetFirstSection
Summary      :  取得第一个段的iter
Input        :
Output       :
Return value :  成功or失败
==============================================================================*/
SectionVecIt CSPIniReadWrite::getFirstSection(void)
{
    return m_ini.begin();
}

/*==============================================================================
Function Name:  GetLastSection
Summary      :  取得最后一个段的iter
Input        :
Output       :
Return value :  成功or失败
==============================================================================*/
SectionVecIt CSPIniReadWrite::getLastSection(void)
{
    return m_ini.end();
}

/*==============================================================================
Function Name:  CompositeSectionString
Summary      :  在指定的字符串的前后各加上"["和"]"
Input        :
Output       :
Return value :
==============================================================================*/
String compositeSectionString(const String& strInput)
{
	String strTmp = "[";
	strTmp += strInput;
	strTmp += "]";
	return strTmp;
}

/*==============================================================================
Function Name:  CompositeKeyString
Summary      :  用=连接key和value
Input        :
Output       :
Return value :
==============================================================================*/
String compositeKeyString(const String& strKey,const String& strValue)
{
	return strKey + "=" + strValue;
}
/*==============================================================================
Function Name:  WriteFile
Summary      :  更新文件
Input        :
Output       :
Return value :
==============================================================================*/
void CSPIniReadWrite::writeFile(const String& pFilePath)
{
	File file(pFilePath);
	if (file.exists() == false)
	{
		file.create();
	}
	ScopedPointer<FileOutputStream> pOutputStream(file.createOutputStream());
	if (pOutputStream == nullptr)
	{
		return;
	}
	pOutputStream->setPosition(0);
	pOutputStream->truncate();
	//写成Unicode格式文件
	pOutputStream->writeByte(0xFF);
	pOutputStream->writeByte(0xFE);

    SectionVecIt iPG = getFirstSection();

    for (; iPG != getLastSection(); iPG++)
    {
		// 写段
		pOutputStream->writeText(compositeSectionString((*iPG)->GetSectionName()), true, false);
		pOutputStream->writeText(pOutputStream->getNewLineString(), true, false);

		// 写键和值
        Key::iterator iKey = (*iPG)->m_tKeys.begin();
        for (; iKey != (*iPG)->m_tKeys.end(); iKey++)
        {
			pOutputStream->writeText(compositeKeyString(iKey->first, iKey->second), true, false);
			pOutputStream->writeText(pOutputStream->getNewLineString(), true, false);
        }
    }
	pOutputStream->flush();
}

/*==============================================================================
Function Name:  SetValue
Summary      :  设置键值，bCreate是指段名及键名未存在时，是否创建。
Input        :
                @lpSection  段名
                @lpKey      键名
                @lpValue    键的新值
                @bCreate    键不存在时是否创建
Output       :
Return value :  成功or失败
==============================================================================*/
bool CSPIniReadWrite::setValue(const String& lpSection, const String& lpKey, const String& lpValue, bool bCreate)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		if (bCreate)
		{
			// 段不存在，全部创建
			m_ini.push_back(new Section(lpSection));
			m_ini.back()->m_tKeys.insert(pair_Key(lpKey, lpValue));
		}
		return false;
	}

	Key::iterator iKEY = (*iPG)->m_tKeys.find(lpKey);
	if (iKEY == (*iPG)->m_tKeys.end())
	{
		// 键不存在，创建键
		if (bCreate)
		{
			// 创建键
			(*iPG)->m_tKeys.insert(pair_Key(lpKey, lpValue));
		}
		return false;
	}
	else
	{
		// 键存在，修改键值
		iKEY->second = lpValue;
		return true;
	}
}

/*==============================================================================
Function Name:  SetValue
Summary      :  设置键值，bCreate是指段名及键名未存在时，是否创建。
Input        :
                @lpSection  段名
                @lpKey      键名
                @lpValue    键的新值
                @bCreate    键不存在时是否创建
Output       :
Return value :  成功or失败
==============================================================================*/
bool CSPIniReadWrite::setValue(const String& lpSection, const String& lpKey, int nValue, bool bCreate)
{
    return setValue(lpSection, lpKey, String(nValue), bCreate);
}

/*==============================================================================
Function Name:  GetFirstSectionName
Summary      :  取得第一个段的iter
Input        :
Output       :
Return value :  名称
==============================================================================*/
String CSPIniReadWrite::getFirstSectionName(void)
{
    if (m_ini.size() == 0)
    {
        return "";
    }

    return (*getFirstSection())->GetSectionName();
}

/*==============================================================================
Function Name:  Update
Summary      :  更新文件
Input        :
Output       :
Return value :
==============================================================================*/
void CSPIniReadWrite::update(void)
{
	// 先写到临时文件，再覆盖原文件
	File file(m_strPath);
	juce::Random r1;
	String strTempFile = m_strPath + String(r1.nextInt());
	writeFile(strTempFile);
	File(strTempFile).moveFileTo(file);
}

/*==============================================================================
Function Name:  GetInt
Summary      :  取得键值
Input        :
                @lpKey      键
                @lpSection  段
Output       :
Return value :  键值（-1表示失败）
==============================================================================*/
int CSPIniReadWrite::getInt(const String& lpSection, const String& lpKey, int nDefaultValue)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return nDefaultValue;
	}

	Key::iterator iKEY = (*iPG)->m_tKeys.find(lpKey);
	if (iKEY == (*iPG)->m_tKeys.end())
	{
		return nDefaultValue;
	}
	return str2Int(iKEY->second);
}

/*==============================================================================
Function Name:  GetString
Summary      :  取得键值
Input        :
                @lpKey      键
                @lpSection  段
Output       :
Return value :  键值（""表示失败）
==============================================================================*/
String CSPIniReadWrite::getString(const String& lpSection, const String& lpKey, const String& lpDefault)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return lpDefault;
	}

	Key::iterator iKEY = (*iPG)->m_tKeys.find(lpKey);
	if (iKEY == (*iPG)->m_tKeys.end())
	{
		return lpDefault;
	}
	return iKEY->second;
}

/*==============================================================================
Function Name:  GetInt
Summary      :  取得键值
Input        :
                @nKey       键
                @lpSection  段
Output       :
Return value :  键值（-1表示失败）
==============================================================================*/
int CSPIniReadWrite::getInt(const String& lpSection, int nKey, int nDefaultValue)
{
    return getInt(lpSection, String(nKey), nDefaultValue);
}

/*==============================================================================
Function Name:  GetString
Summary      :  取得键值
Input        :
                @nKey       键
                @lpSection  段
Output       :
Return value :  键值（""表示失败）
==============================================================================*/
String CSPIniReadWrite::getString(const String& lpSection, int nKey, const String& lpDefault)
{
    return getString(lpSection, String(nKey), lpDefault);
}

bool CSPIniReadWrite::isSectionExist(const String& lpSection)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return false;
	}
	return true;
}
/*==============================================================================
Function Name:  IsKeyExist
Summary      :  判断key是否存在
Input        :
Output       :
Return value :  段的数目
==============================================================================*/
bool CSPIniReadWrite::isKeyExist(const String& lpSection, const String& lpKey)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return false;
	}

	Key::iterator iKEY = (*iPG)->m_tKeys.find(lpKey);
	if (iKEY == (*iPG)->m_tKeys.end())
	{
		return false;
	}
	return true;
}

/*==============================================================================
Function Name:  GetSectionNumber
Summary      :  段的数目
Input        :
Output       :
Return value :  段的数目
==============================================================================*/
int CSPIniReadWrite::getSectionNumber(void)
{
    return m_ini.size();
}

/*==============================================================================
Function Name:  GetKeyNumber
Summary      :  取得指定段的键的数目
Input        :
                @lpSection 段
Output       :
Return value :  键的数目
==============================================================================*/
int CSPIniReadWrite::getKeyNumber(const String& lpSection)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return 0;
	}
	// 找到了
     return (*iPG)->m_tKeys.size();
}

/*==============================================================================
Function Name:  Str2Int
Summary      :  字符串转换为数字
Input        :
                @a_strInput 字符串
Output       :
Return value :  数字
==============================================================================*/
int CSPIniReadWrite::str2Int(const String& str)
{
    return str.getIntValue();
}
//int to string
/*==============================================================================
Function Name:  Int2Str
Summary      :  数字转换为字符串
Input        :
                @a_nVal 数字
Output       :
Return value :  字符串
==============================================================================*/
String CSPIniReadWrite::int2Str(int nVal)
{
    return String(nVal);
}

/*==============================================================================
Function Name:  SetFileName
Summary      :  设置文件路径
Input        :
                @strFileName 文件路径
Output       :
Return value :
==============================================================================*/
void CSPIniReadWrite::setFileName(const String& strFileName)
{
	m_strPath = strFileName;
	if (isFileExist(strFileName))
	{
		delAllSections();
		readFile();
	}
	else
	{
		//LogError(L"IniReadWrite", "%s is not existed!", lpFileName);
	}
}

bool CSPIniReadWrite::loadFromBuffer(byte* pBuffer, uint length)
{
	if (pBuffer)
	{
		delAllSections();

		uint i = 0;
		uint start = 0;
		uint end = 0;

		String strOneLine;				// 一行文本内容
		String strKey;					// 键名
		String strValue;				// 键值
		Section *myPG = nullptr;		// 段的iter
		int squarebrackets_left = -1;	// 左中括号的位置
		int squarebrackets_right = - 1;	// 右中括号的位置
		int equalsymbol = -1;			// 等号的位置

		//unicode16
		if (length >= 2 && pBuffer[0] == 0xFF && pBuffer[1] == 0xFE)
		{
			CharPointer_UTF16::CharType byTemp;
			CharPointer_UTF16::CharType* pTBuffer = (CharPointer_UTF16::CharType*)(pBuffer + 2);
			length = (length - 2)/2;
// 			i = 2;
// 			start = i;
			while (1)
			{
				byTemp = pTBuffer[i];
				if (byTemp != 0 && byTemp != L'\r' && byTemp != L'\n' && i < length)
				{
					i++;
				}
				else
				{
					end = i;
					if (start == end)
					{
						if (byTemp == 0 || i >= length)
						{
							break;
						}
						else
						{
							start = end + 1;
							i = start;
							continue;
						}
					}
					//处理一行数据
					strOneLine = String(CharPointer_UTF16(pTBuffer + start), CharPointer_UTF16(pTBuffer + end)).trim();

					// 判断是否是空行
					if (strOneLine.isEmpty())
					{
						if (byTemp == 0 || i >= length)
						{
							break;
						}
						else
						{
							start = end + 1;
							i = start;
							continue;
						}
					}

					squarebrackets_left = strOneLine.indexOfChar(L'[');
					squarebrackets_right = strOneLine.indexOfChar(L']');

					// 以下处理建
					if ((squarebrackets_left != -1) && (squarebrackets_right != -1) && (squarebrackets_left < squarebrackets_right))
					{
						int equalPos = strOneLine.indexOfChar(L'=');
						if (equalPos == -1 || (equalPos > squarebrackets_left && equalPos < squarebrackets_right))
						{
							strKey = strOneLine.substring(squarebrackets_left + 1, squarebrackets_right);

							myPG = new Section(strKey);
							m_ini.push_back(myPG);

							if (byTemp == 0 || i >= length)
							{
								break;
							}
							else
							{
								start = end + 1;
								i = start;
								continue;
							}
						}
					}

					// 以下处理key
					equalsymbol = strOneLine.indexOfChar(L'=');
					strKey = strOneLine.substring(0, equalsymbol).trim();	// 取得键名

					// 取得键值
					strValue = strOneLine.substring(equalsymbol + 1, strOneLine.length()).trim();

					m_ini.back()->m_tKeys.insert(pair_Key(strKey, strValue));


					if (byTemp == 0 || i >= length)
					{
						break;
					}
					start = end + 1;
					i = start;
				}
			}
		}
		//UTF8
		else if (length >= 3 && pBuffer[0] == 0xEF && pBuffer[1] == 0xBB && pBuffer[2] == 0xBF)
		{
			i = 3;// 要先减去标志位
			start = i;
			CharPointer_UTF8::CharType byTemp;

			while (1)
			{
				byTemp = pBuffer[i];
				if (byTemp != 0 && byTemp != '\r' && byTemp != '\n' && i < length)
				{
					i++;
				}
				else
				{
					end = i;
					if (start == end)
					{
						if (byTemp == 0 || i >= length)
						{
							break;
						}
						else
						{
							start = end + 1;
							i = start;
							continue;
						}
					}
					//处理一行数据
					strOneLine = String(CharPointer_UTF8((CharPointer_UTF8::CharType*)(pBuffer + start)),
						CharPointer_UTF8((CharPointer_UTF8::CharType*)(pBuffer + end))).trim();

					// 判断是否是空行
					if (strOneLine.isEmpty())
					{
						if (byTemp == 0 || i >= length)
						{
							break;
						}
						else
						{
							start = end + 1;
							i = start;
							continue;
						}
					}

					squarebrackets_left = strOneLine.indexOfChar(L'[');
					squarebrackets_right = strOneLine.indexOfChar(L']');

					// 以下处理建
					if ((squarebrackets_left != -1) && (squarebrackets_right != -1) && (squarebrackets_left < squarebrackets_right))
					{
						int equalPos = strOneLine.indexOfChar(L'=');
						if (equalPos == -1 || (equalPos > squarebrackets_left && equalPos < squarebrackets_right))
						{
							strKey = strOneLine.substring(squarebrackets_left + 1, squarebrackets_right);
							myPG = new Section(strKey);
							m_ini.push_back(myPG);

							if (byTemp == 0 || i >= length)
							{
								break;
							}
							else
							{
								start = end + 1;
								i = start;
								continue;
							}
						}
					}

					// 以下处理key
					equalsymbol = strOneLine.indexOfChar(L'=');

					strKey = strOneLine.substring(0, equalsymbol).trim();	// 取得键名

					// 取得键值
					strValue = strOneLine.substring(equalsymbol + 1, strOneLine.length()).trim();

					m_ini.back()->m_tKeys.insert(pair_Key(strKey, strValue));

					if (byTemp == 0 || i >= length)
					{
						break;
					}
					start = end + 1;
					i = start;
				}
			}
		}
		else
		{
			CharPointer_ASCII::CharType byTemp;

			while (1)
			{
				byTemp = pBuffer[i];
				if (byTemp != 0 && byTemp != '\r' && byTemp != '\n' && i < length)
				{
					i++;
				}
				else
				{
					end = i;
					if (start == end)
					{
						if (byTemp == 0 || i >= length)
						{
							break;
						}
						else
						{
							start = end + 1;
							i = start;
							continue;
						}
					}
					//处理一行数据
					strOneLine = String((CharPointer_ASCII::CharType*)(pBuffer + start), end - start).trim();

					// 判断是否是空行
					if (strOneLine.isEmpty())
					{
						if (byTemp == 0 || i >= length)
						{
							break;
						}
						else
						{
							start = end + 1;
							i = start;
							continue;
						}
					}

					squarebrackets_left = strOneLine.indexOfChar(L'[');
					squarebrackets_right = strOneLine.indexOfChar(L']');

					// 以下处理建
					if ((squarebrackets_left != -1) && (squarebrackets_right != -1) && (squarebrackets_left < squarebrackets_right))
					{
						int equalPos = strOneLine.indexOfChar(L'=');
						if (equalPos == -1 || (equalPos > squarebrackets_left && equalPos < squarebrackets_right))
						{
							strKey = strOneLine.substring(squarebrackets_left + 1, squarebrackets_right);
							myPG = new Section(strKey);
							m_ini.push_back(myPG);

							if (byTemp == 0 || i >= length)
							{
								break;
							}
							else
							{
								start = end + 1;
								i = start;
								continue;
							}
						}
					}

					// 以下处理key
					equalsymbol = strOneLine.indexOfChar(L'=');
					if (equalsymbol != -1)
					{
						strKey = strOneLine.substring(0, equalsymbol).trim();	// 取得键名

						// 取得键值
						strValue = strOneLine.substring(equalsymbol + 1, strOneLine.length()).trim();

						m_ini.back()->m_tKeys.insert(pair_Key(strKey, strValue));
					}

					if (byTemp == 0 || i >= length)
					{
						break;
					}
					start = end + 1;
					i = start;
				}
			}
		}

		return true; 
	}
	return false;
}

/*==============================================================================
Function Name:  ReadFile
Summary      :  读取ini文件的内容
Input        :
Output       :
Return value :
==============================================================================*/

void CSPIniReadWrite::readFile(void)
{
	File file(m_strPath);
	if (file.existsAsFile())
	{
		ScopedPointer<FileInputStream> pInputStream(file.createInputStream());
		if (pInputStream)
		{
			size_t nSize = static_cast<size_t>(file.getSize());
			MemoryBlock memoryBlock(nSize + 1, true);
			if (pInputStream->read(memoryBlock.getData(), nSize) == nSize)
			{
				loadFromBuffer((byte*)memoryBlock.getData(), nSize);
			}
		}
	}
}

/*==============================================================================
Function Name:  GetFileName
Summary      :  取得ini文件路径
Input        :
Output       :
Return value :  ini文件路径
==============================================================================*/
const String& CSPIniReadWrite::getFileName(void)
{
    return m_strPath;
}

/*==============================================================================
Function Name:  DelSection
Summary      :  删除section段
Input        :
                @lpSection  ：【】中的名字
Output       :
Return value :  成功or失败
==============================================================================*/
bool CSPIniReadWrite::delSection(const String& lpSection)  //删除段名
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return false;
	}

    // 找到了
    //SAFE_DELETE(*iPG);
    m_ini.erase(iPG);
    return true;
}

/*==============================================================================
Function Name:  DelKey
Summary      :  删除字段
Input        :
                @lpSection  ：【】中的名字
                @LPCTSTR lpKey,     ：=左边的字段名
Output       :
Return value :  成功or失败
==============================================================================*/
bool CSPIniReadWrite::delKey(const String& lpSection, const String& lpKey)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return false;
	}
	Key::iterator iKEY = (*iPG)->m_tKeys.find(lpKey);
	if (iKEY == (*iPG)->m_tKeys.end())
	{
		return false;
	}
	// 找到了
	(*iPG)->m_tKeys.erase(iKEY);
	return true;
}

/*==============================================================================
Function Name:  ModifyKey
Summary      :  修改指定的键名
Input		 :
Output       :
Return value :   
==============================================================================*/
bool CSPIniReadWrite::modifyKey(const String& lpSection, const String& lpKey, const String& lpNewKey)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return false;
	}
	
	Key::iterator iKEY = (*iPG)->m_tKeys.find(lpKey);
	if (iKEY == (*iPG)->m_tKeys.end())
	{
		return false;
	}
	
	// 找到了,先删除该键，然后插入新建
	// 保存值
	(*iPG)->m_tKeys.insert(make_pair(lpKey, lpNewKey));
	return true;
}

/*==============================================================================
Function Name:  GetAllSections
Summary      :  取出所有的section段
Input		 :
Output       :
				@arrSection所有的段名集合
Return value :    所有的段的个数
==============================================================================*/
int CSPIniReadWrite::getAllSections(vector<String>& arrSection)
{
	SectionVecIt iPG = getFirstSection();

	for (; iPG != getLastSection(); iPG++)
	{
		arrSection.push_back((*iPG)->GetSectionName());
	}
	return arrSection.size();
}

/*==============================================================================
Function Name:  GetAllKeysAndValues
Summary      :  取出制定的section段的所有的key以及key对应的值
Input		 :
				@lpSection指定的section名
Output       :
				@arrKey所有的=左边的key的集合
				@arrValue所有的=右边的key的值的集合
Return value :    key的个数
==============================================================================*/
int CSPIniReadWrite::getAllKeysAndValues(vector<String>& arrKey, vector<String>& arrValue, const String& lpSection)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return -1;
	}

	// 找到了段,取得该段所有的键和值
	Key::iterator iKEY = (*iPG)->m_tKeys.begin();

	for (; iKEY != (*iPG)->m_tKeys.end(); iKEY++)
	{
		arrKey.push_back(iKEY->first);
		arrValue.push_back(iKEY->second);
	}
	return arrKey.size();
}


/*==============================================================================
Function Name:  DelAllSections
Summary      :  删除所有的段
Input        :
Output       :
Return value :
==============================================================================*/
void CSPIniReadWrite::delAllSections()
{
	//DelObj(m_ini);
}

/*==============================================================================
Function Name:  FindSection
Summary      :  查找指定的段
Input        :
Output       :
Return value :
==============================================================================*/
SectionVecIt CSPIniReadWrite::findSection(const String& lpSection)
{
	SectionVecIt iPG = getFirstSection();

	for (; iPG != getLastSection(); iPG++)
	{
		if ((*iPG)->GetSectionName() == lpSection)
		{
			break;
		}
	}
	return iPG; // 未找到的时候返回的是GetLastSection;
}

/*==============================================================================
Function Name:  FindKeyBySectionAndValude
Summary      :  通过指定的段和某个键的值来查找该键
Input        :
Output       :
Return value :
==============================================================================*/
String CSPIniReadWrite::findKeyBySectionAndValude(const String& lpSection, const String& lpValue)
{
	SectionVecIt iPG = findSection(lpSection);
	if (iPG == getLastSection())
	{
		return "";
	}

	Key::iterator iKEY = (*iPG)->m_tKeys.begin();

	for (; iKEY != (*iPG)->m_tKeys.end(); iKEY++)
	{
		if (iKEY->second != lpValue)
		{
			break;
		}
	}
	return iKEY->first;
}

//***************************************************************
// *  功　　能: 判断文件是否存在
// *  入口参数:
// *  出口参数:
// *  返 回 值:
// *  作　　者:
// *  日　　期: 
//***************************************************************
bool CSPIniReadWrite::isFileExist(const String& strPath)
{
    return File(strPath).existsAsFile();
}