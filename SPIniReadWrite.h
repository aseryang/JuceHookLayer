#ifndef _SP_INI_READ_WRITE_H_
#define _SP_INI_READ_WRITE_H_

//#include "../Helper/SPDefine.h"
//#include "../JuceLibraryCode/modules/juce_core/juce_core.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "map"
#include "vector"

using namespace std;
using namespace juce;

#define TRANSPARENTLEVEL		16
#define PI		3.141592653589793f
#define PI_2	1.570796326794896f
#define DEGREE	PI/180.0

#define min4(x1, x2, x3, x4)	min(min(x1, x2), min(x3, x4))
#define max4(x1, x2, x3, x4)	max(max(x1, x2), max(x3, x4))
#define clamp(value, minValue, maxValue)			((value)<(minValue)?(minValue):((value)>(maxValue)?(maxValue):(value)))

typedef unsigned char byte;
typedef unsigned int  uint;
#define BUF_SIZE 256
typedef std::map<String, String> Key;
typedef std::pair<String, String> pair_Key;

// 段
struct Section
{
	String	m_strSecName;
	Key		m_tKeys;

	Section(const String& app)
	{
		m_strSecName = app;
	}
	String GetSectionName(void)
	{
		return m_strSecName;
	}
};

typedef std::vector<Section*> SectionVec;
typedef std::vector<Section*>::iterator SectionVecIt;

//==============================================================================
/** Ini文件的读写操作类
*/

class JUCE_API CSPIniReadWrite
{
public:
	CSPIniReadWrite();

	CSPIniReadWrite(const String& strFileName)
	{
		setFileName(strFileName);
	}
	~CSPIniReadWrite();

	/** 设置键值，bCreate是指段名及键名未存在时，是否创建 */
	bool setValue(const String& lpSection, const String& lpKey, const String& lpValue,bool bCreate=true); 

	/** 设置键值，bCreate是指段名及键名未存在时，是否创建 */
	bool setValue(const String& lpSection, const String& lpKey, int nValue,bool bCreate=true); 

	/** 取得键值 */
	int getInt(const String& lpSection, const String& lpKey, int nDefaultValue = -1);

	/** 取得键值 */
	String getString(const String& lpSection, const String& lpKey, const String& lpDefault = String::empty);

	/** 判断指定段是否存在 */
	bool isSectionExist(const String& lpSection);

	/** 判断key是否存在 */
	bool isKeyExist(const String& lpSection, const String& lpKey);

	/** 取得键值 */
	int getInt(const String& lpSection, int nKey, int nDefaultValue = -1);

	/** 取得键值 */
	String getString(const String& lpSection, int nKey, const String& lpDefault = String::empty);

	/** 删除指定键 */
	bool delKey(const String& lpSection, const String& lpKey); 

	// 修改指定的键名 */
	bool modifyKey(const String& lpSection, const String& lpKey, const String& lpNewKey); 

	/** 删除指定段 */
	bool delSection(const String& lpSection); 

	/** 更新文件 */
	void update(void);
	
	/** 取得段数目 */
	int getSectionNumber(void);
	/** 取得指定段的键数目 */
	int getKeyNumber(const String& lpSection);
	/** */
	String findKeyBySectionAndValude(const String& lpSection,const String& lpValue);

	void setFileName(const String& strFileName);  

	/** 获得文件名 */
	const String& getFileName(void);
	/** 取得第一个段的名字 */
	String getFirstSectionName(void);
	/** 从内存流加载配置文件 */
	bool loadFromBuffer(byte* pBuffer, uint length);

	/** 枚举出全部的段名 */
	int getAllSections(vector<String>& arrSection);

	/**　枚举出一段内的全部键名及值 */
	int getAllKeysAndValues(vector<String>& arrKey,vector<String>& arrValue,const String& lpSection);

	/** 删除所有的段 */
	void delAllSections(void);

	/** 把配置信息写入文件中 */
	void writeFile(const String& pFilePath);
private:
	// 文件是否存在
	bool isFileExist(const String& strPath);

	// 读取文件中的信息并解析
	void readFile(void);
	// 字符串转换为数字
	int str2Int(const String& strInput);
	// 数字转换为字符串
	String int2Str(int nVal);
	// 取第一个段的iter
	SectionVecIt getFirstSection(void);
	// 取最后一个段的iter
	SectionVecIt getLastSection(void);
	// 查找section
	SectionVecIt findSection(const String& lpSection);

private:
	SectionVec m_ini;
	String m_strPath;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CSPIniReadWrite);
};

#endif