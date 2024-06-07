import os, sys, getopt
import xml.etree.ElementTree as ET
import re

CONFIG_FILE = '"./Config.xml"'
MSG_PROC_FILE_NAME = 'DispCtrl'
CLASS_NAME = 'C'+MSG_PROC_FILE_NAME

SKIP_NODE_FLAG = 'SkipNodeFlag'

SET_OPERATE_DECAL_HEAD = '#ifndef '+MSG_PROC_FILE_NAME.upper()+'_H_\n#define '+MSG_PROC_FILE_NAME.upper()+'_H_\n'+'''

#include <vector>
#include <string>

#include <kanzi/kanzi.hpp>

#include "DispCtrlTypedef.h"

using namespace kanzi;

#ifdef DATA_SOURCE
typedef DataObjectSharedPtr RootTypeSharedPtr;
#else
typedef Node2DSharedPtr RootTypeSharedPtr;
#endif

class ClassName
{
public:
	ClassName(RootTypeSharedPtr oRoot) : m_oRoot(oRoot)
	{}

	~ClassName()
	{
		m_oRoot.reset();
	}

'''

SET_OPERATE_DECAL_TAIL = '''

private:
#ifdef DATA_SOURCE
	RootTypeSharedPtr GetNode(const std::string& strPath);
#else
	NodeSharedPtr GetNode(const std::string& strPath);
#endif
	
	template <typename TagValueType>
	bool SetConfig(const std::string& strPath, TagValueType tValue);

private:
	RootTypeSharedPtr m_oRoot;
};

#endif
'''

SET_OPERATE_DEF_HEAD = '#include "'+MSG_PROC_FILE_NAME+'.h"'+'''

static std::vector<std::string> StringSplit(const std::string& strDst, char cSpilt)
{
	std::vector<std::string>vtrStrRet;
	std::string strTmp;
	for (unsigned int i = 0; i < strDst.length(); ++i)
	{
		if (cSpilt == strDst[i])
		{
			if (!strTmp.empty())
			{
				vtrStrRet.push_back(strTmp);
				strTmp.clear();
			}
		}
		else
		{
			strTmp += strDst[i];
		}
	}

	if (!strTmp.empty())
	{
		vtrStrRet.push_back(strTmp);
		strTmp.clear();
	}

	return vtrStrRet;
}

#ifdef DATA_SOURCE
RootTypeSharedPtr ClassName::GetNode(const std::string& strPath)
{
	std::vector<std::string> vtrPath = StringSplit(strPath, '/');
	std::vector<std::string>::iterator it = vtrPath.begin();
	RootTypeSharedPtr oNode = m_oRoot;
	for (; vtrPath.end() != it; ++it)
	{
		oNode = oNode->findChild((*it).c_str());
		if (NULL == oNode.get())
		{
			break;
		}
	}

	return oNode;
}

template <>
bool ClassName::SetConfig(const std::string& strPath, bool bValue)
{
	RootTypeSharedPtr oNode = GetNode(strPath);
	if (NULL != oNode.get())
	{
		((DataObjectBool *)oNode.get())->setValue(bValue);
		return true;
	}
	
	return false;
}

template <>
bool ClassName::SetConfig(const std::string& strPath, int iValue)
{
	RootTypeSharedPtr oNode = GetNode(strPath);
	if (NULL != oNode.get())
	{
		((DataObjectInt *)oNode.get())->setValue(iValue);
		return true;
	}
	
	return false;
}

template <>
bool ClassName::SetConfig(const std::string& strPath, float fValue)
{
	RootTypeSharedPtr oNode = GetNode(strPath);
	if (NULL != oNode.get())
	{
		((DataObjectReal *)oNode.get())->setValue(fValue);
		return true;
	}
	
	return false;
}

template <>
bool ClassName::SetConfig(const std::string& strPath, double fValue)
{
	RootTypeSharedPtr oNode = GetNode(strPath);
	if (NULL != oNode.get())
	{
		((DataObjectReal *)oNode.get())->setValue(fValue);
		return true;
	}
	
	return false;
}

template <>
bool ClassName::SetConfig(const std::string& strPath, const std::string& strValue)
{
	RootTypeSharedPtr oNode = GetNode(strPath);
	if (NULL != oNode.get())
	{
		((DataObjectString *)oNode.get())->setValue(strValue);
	}
}
#else
NodeSharedPtr CDispCtrl::GetNode(const std::string& strPath)
{
	return m_oRoot->lookupNode<Node>(strPath.c_str());
}

template <typename TagValueType>
bool ClassName::SetConfig(const std::string& strPath, TagValueType tValue)
{
	std::string::size_type iPropertyBeginIndex = strPath.rfind('/');
	if (std::string::npos != iPropertyBeginIndex)
	{
		std::string strProperty = strPath.substr(iPropertyBeginIndex + 1);
		std::string strNodePath = strPath.substr(0, iPropertyBeginIndex - 1);
		std::string::size_type iNodeAliasBeginIndex = strNodePath.rfind('/');
		if (std::string::npos != iPropertyBeginIndex)
		{
			std::string strNodeAlias = '#' + strNodePath.substr(0, iNodeAliasBeginIndex + 1);
			NodeSharedPtr oNode = GetNode(strNodeAlias);
			if (NULL != oNode.get())
			{
				oNode->setProperty(DynamicPropertyType<TagValueType>(strProperty), tValue);
				return true;
			}
		}
	}
	
	return false;
}
#endif

'''


def HasAttr(node, attr):
	if attr in node.attrib:
		return True
	else:
		return False
		
def HasText(node):
	if not node.text.isspace():
		return True
	else:
		return False

def IsStruct(node):
	if HasAttr(node, 'type'):
		if node.attrib['type']=='struct':
			return True
		
	return False
	
def IsInteger(node):
	if HasAttr(node, 'type'):
		if node.attrib['type']=='int':
			return True
	return False
	
def IsFloat(node):
	if HasAttr(node, 'type'):
		if node.attrib['type']=='float':
			return True
	return False
	
def IsBoolean(node):
	if HasAttr(node, 'type'):
		if node.attrib['type']=='bool':
			return True
	return False
	
def IsString(node):
	if HasAttr(node, 'type'):
		if node.attrib['type']=='string':
			return True
	return False
	
def IsList(node):
	if HasAttr(node, 'type'):
		if node.attrib['type']=='list':
			return True
	return False
	
def	IsDataStruct(node):
	if IsInteger(node) or IsFloat(node) or IsBoolean(node) or IsString(node) or IsList(node) or IsStruct(node):
		return True
	return False
	
def GenerateIdentify(node, nodePath):
	if nodePath != SKIP_NODE_FLAG:
		nodePath += node.tag+'_'
	else:
		nodePath = ''

	str=''
	getIdentify = False
	for child in node:
		str += GenerateIdentify(child, nodePath)
		if HasAttr(child, 'identify'):
			getIdentify = True
			if HasAttr(node, 'identifyType'):
				str += 'const '+node.attrib['identifyType']+' '
			else:
				str += 'const char* '
			str += nodePath.upper()+'ID'
			strList = re.findall('[A-Z][a-z]*', child.tag)
			for strWord in strList:
				str += '_'+strWord.upper()
			str += ' = '+child.attrib['identify']+';\n'
			
	if getIdentify:
		str += '\n'
		
	return str
	
def GetTypeStr(node):
	if IsDataStruct(node):
		if IsString(node):
			return 'std::string'
		elif IsStruct(node):
			if HasAttr(node, 'stuName'):
				return 'Stu'+node.attrib['stuName']
			else:
				return 'Stu'+node.tag
		elif IsList(node):
			dataNode = node[0]
			while not HasAttr(dataNode, 'type'):
				dataNode = dataNode[0]
			if HasAttr(dataNode, 'identify'):
				str = 'std::vector< std::pair<'
				if HasAttr(node, 'identifyType'):
					return str+node.attrib['identifyType']+', '+GetTypeStr(dataNode)+'> >'
				else:
					return str+'std::string, '+GetTypeStr(dataNode)+'> >'
			else:
				return 'std::vector<'+GetTypeStr(dataNode)+'>'
		else:
			return node.attrib['type']
			
	return ''
	
def GetParamNameStr(node):
	if IsDataStruct(node):
		if IsString(node):
			return 'str'+node.tag
		elif IsStruct(node):
			if HasAttr(node, 'stuName'):
				return 'st'+node.attrib['stuName']
			else:
				return 'st'+node.tag
		elif IsList(node):
			return 'vtr'+node.tag
		elif IsBoolean(node):
			return 'b'+node.tag
		elif IsInteger(node):
			return 'i'+node.tag
		elif IsFloat(node):
			return 'f'+node.tag
		else:
			return ''
			
	return ''
	
def GetStructMemberParam(node):
	strRet = ''
	if IsDataStruct(node):
		strRet = GetTypeStr(node)+' '+GetParamNameStr(node)
			
	return strRet
	
def GetFuncParam(node):
	strRet = ''
	if IsDataStruct(node):
		if IsString(node) or IsStruct(node) or IsList(node):
			strRet = GetTypeStr(node)+'& '+GetParamNameStr(node)
		else:
			strRet = GetTypeStr(node)+' '+GetParamNameStr(node)
			
	return strRet
	
def GetFuncConstParam(node):
	strRet = ''
	if IsDataStruct(node):
		strRet = 'const '+GetFuncParam(node)
			
	return strRet
	
def GenerateStruct(node):
	str=''
	if IsStruct(node):
		structStr = 'struct '+GetTypeStr(node)+'\n{'
		
	for child in node:
		tmp = GenerateStruct(child)
		if tmp not in str:
			str += tmp
			
		if IsStruct(node):
			if HasAttr(child, 'type'):
				structStr += '\n'+GetStructMemberParam(child)+';'
			else:
				print("Child node must has 'type' attr when it's parent has 'strcut' type attr")
			
	if IsStruct(node):
		structStr += '\n};\n\n'
		str += structStr

	return str
	
def GenerateMsgInterface(node):
	str=''
	for child in node:
		if IsDataStruct(child):
			msgInterfaceStr = 'INTERFACE_BEGIN(IDisp'+child.tag+', "/pps/ZH/HMI/")\nMETHOD_V1(void, UpdateDisp'+child.tag+', '
			if IsString(child) or IsStruct(child) or IsList(child):
				msgInterfaceStr += 'const '+GetTypeStr(child)+'&'
			else:
				msgInterfaceStr += GetTypeStr(child)
			msgInterfaceStr += ')\n'+'INTERFACE_END(IDisp'+child.tag+')\n\n'
			if msgInterfaceStr not in str:
				str += msgInterfaceStr
		else:
			str += GenerateMsgInterface(child)
		
	return str
	
def GenerateStructCodecFunc(node):
	str=''
	if IsStruct(node):
		structStr = 'template <typename Archive>\nstatic void serialize(Archive &ar, '+GetFuncParam(node)+')\n{'
		
	for child in node:
		tmp = GenerateStructCodecFunc(child)
		if tmp not in str:
			str += tmp
			
		if IsStruct(node):
			structStr += '\nar & '+GetParamNameStr(node)+'.'+GetParamNameStr(child)+';'
			
	if IsStruct(node):
		structStr += '\n};\n\n'
		str += structStr
			
	return str
	
def GenerateMsgProcFuncDeclaration(node):
	str=''
	for child in node:
		if IsDataStruct(child):
			funcStr = 'void UpdateDisp'+child.tag+'('
			if IsString(child) or IsStruct(child) or IsList(child):
				funcStr += GetFuncConstParam(child)
			else:
				funcStr += GetFuncParam(child)
			funcStr += ');\n'
			if funcStr not in str:
				str += funcStr
		else:
			str += GenerateMsgProcFuncDeclaration(child)
			
	return str
	
def GetSetConfigOperation(node, nodePath):
	nodePath += '/'+node.tag
	strOperation = 'SetConfig("' +nodePath+'", '+GetParamNameStr(node)+');\n'
		
	return strOperation
	
def GetStructSetConfigOperation(node, nodePath):
	strOperation = ''
	nodePath += '/'+node.tag
	for child in node:
		strOperation += 'oConfig.SetConfig("' +nodePath+'/'+child.tag+'", '+GetParamNameStr(node)+'.'+GetParamNameStr(child)+');\n'

	return strOperation
	
def GetListSetConfigOperation(node, nodePath):
	strOperation = ''
	
	mapDefinition = False
	nodePath += '/'+node.tag
	for child in node:
		tagPath = nodePath
		dataNode = child
		dataParentNode = node
		tagPath += '/'+dataNode.tag
		while not HasAttr(dataNode, 'type'):
			dataParentNode = dataNode
			dataNode = dataNode[0]
			tagPath += '/'+dataNode.tag
			
		if HasAttr(dataNode, 'identify'):
			if not mapDefinition:
				mapDefinition = True
				strOperation = 'static std::map<'
				strOperation += dataParentNode.attrib['identifyType']+', std::string> mapPath;\n'
				strOperation += 'static bool bFirst = true;\nif(bFirst)\n{\nbFirst = false;\n'
					
			strOperation += 'mapPath.insert(std::make_pair('+dataNode.attrib['identify']+', "'+tagPath+'"));\n'
	
	if mapDefinition:
		strOperation += '}\n\n'
		strOperation +=  'auto itTagPath = mapPath.end();\n'
	
	dataNode = node[0]
	nodePath += '/'+dataNode.tag
	while not HasAttr(dataNode, 'type'):
		dataNode = dataNode[0]
		nodePath += '/'+dataNode.tag
		
	strOperation += 'auto it = '+GetParamNameStr(node)+'.begin();\n'
	strOperation += 'for(; '+GetParamNameStr(node)+'.end() != it; ++it)\n{\n'
	if mapDefinition:
		strOperation += 'itTagPath = mapPath.find((*it).first'+');\n'
		strOperation += 'if(mapPath.end() != itTagPath)\n{\n'
		if IsStruct(dataNode):
			for child in dataNode:
				strOperation += 'SetConfig(std::string((*itTagPath).second)+"/'+child.tag+'", '
				strOperation +='(*it).second.'+GetParamNameStr(child)+');\n'
		else:
			strOperation += 'SetConfig((*itTagPath).second, '+'(*it).second'+');\n'
		strOperation += '}\n'
	else:
		if IsStruct(dataNode):
			for child in dataNode:
				strOperation += 'SetConfig("' +nodePath+'/'+child.tag+'", '+'(*it).'+GetParamNameStr(child)+');\n'
		else:
			strOperation += 'SetConfig("' +nodePath+'/'+child.tag+'", '+'(*it).'+GetParamNameStr(child)+');\n'
				
	strOperation += '}\n'
	return strOperation
	
def GenerateMsgProcFuncDefinition(node, nodePath):
	# print('\t'*tabNum, end='')
	# print(node.tag, end='')
	# for itemKey, itemValue in node.attrib.items():
		# print('{'+itemKey+';'+itemValue+'}', end='')
	# if not node.text.isspace():
		# print('[',node.text,']', end='')
	# print()
	str=''
	nodePath += '/'+node.tag
	for child in node:
		if IsDataStruct(child):
			funcStr = 'void '+CLASS_NAME+'::UpdateDisp'+child.tag+'('
			if IsString(child) or IsStruct(child) or IsList(child):
				funcStr += GetFuncConstParam(child)+')'+'\n{\n'
			else:
				funcStr += GetFuncParam(child)+')'+'\n{\n'
			if IsInteger(child) or IsBoolean(child) or IsFloat(child):
				funcStr += GetSetConfigOperation(child, nodePath)
			elif IsString(child):
				funcStr += GetSetConfigOperation(child, nodePath)
			elif IsStruct(child):
				funcStr += GetStructSetConfigOperation(child, nodePath)
			elif IsList(child):
				funcStr += GetListSetConfigOperation(child, nodePath)
			funcStr += '}\n\n'
			if funcStr not in str:
				str += funcStr
		else:
			str += GenerateMsgProcFuncDefinition(child, nodePath)
		
	return str
	
def FormatCode(strCode):
	tabAddKeyWord = '{'
	tabDecKeyWord = '}'
	tabAddOnceLineKeyWord = ['if(', 'else(', 'while(', 'for(', 'switch(']
	tabDecOnceLineKeyWord = ['public:', 'private:', 'protect:']
	
	tabCount = 0
	tabAddOnceLineCount = 0
	tabDecOnceLineCount = 0
	strRet = ''
	for strLine in strCode.splitlines(True):
		strLine = strLine.lstrip(' ')
		strLine = strLine.lstrip('\t')
	
		if tabAddKeyWord in strLine:
			tabAddOnceLineCount = 0
	
		tabAddKeyWordCount = strLine.count(tabAddKeyWord)
		tabDecKeyWordCount = strLine.count(tabDecKeyWord)
		if tabDecKeyWordCount > 0 and tabDecKeyWordCount > tabAddKeyWordCount:
			tabAddOnceLineCount = 0
			tabCount = tabCount+tabAddKeyWordCount
			if tabCount > tabDecKeyWordCount:
				tabCount = tabCount-tabDecKeyWordCount
			else:
				tabCount = 0
				
		for strKeyWord in tabDecOnceLineKeyWord:
			if strKeyWord in strLine:
				tabDecOnceLineCount = tabDecOnceLineCount+1

		strRet += '\t'*(tabCount+tabAddOnceLineCount-tabDecOnceLineCount)+strLine
		tabAddOnceLineCount = 0
		tabDecOnceLineCount = 0
		
		if tabAddKeyWordCount > 0 and tabAddKeyWordCount > tabDecKeyWordCount:
			tabCount = tabCount+tabAddKeyWordCount
			if tabCount > tabDecKeyWordCount:
				tabCount = tabCount-tabDecKeyWordCount
			else:
				tabCount = 0
				
		for strKeyWord in tabAddOnceLineKeyWord:
			if strKeyWord in strLine:
				tabAddOnceLineCount = tabAddOnceLineCount+1
				
	return strRet

if ( __name__ == "__main__"):
	print("Code generator startup\n")
	
	cfgFile = 'Cluster.xml'
	outputPath = os.path.dirname(sys.argv[0])+'\\'
	try:
		opts, args = getopt.getopt(sys.argv[1:], "hi:",["cfgFile="])
	except getopt.GetoptError:
		print('CodeGenerator.py -i <cfgFile>')
		sys.exit(2)
	for opt, arg in opts:
		if opt == '-h':
			print('CodeGenerator.py -i <cfgFile>')
			sys.exit()
		elif opt in ("-i", "--ifile"):
			cfgFile = arg
	
	tree = ET.parse(cfgFile)
	root = tree.getroot()
	#ET.dump(root)
	
	strHead = '#ifndef IDENTIFY_DEFINE_H_\n#define IDENTIFY_DEFINE_H_\n\n'
	strTail = '\n#endif'
	identifyDef = strHead+GenerateIdentify(root, SKIP_NODE_FLAG)+strTail
	identifyDef = FormatCode(identifyDef)
	print(identifyDef, end='\n\n')
	outputPath = "..\\include\\"
	identifyDefFile = open(outputPath+'DispCtrlIdentify.h', 'w')
	identifyDefFile.write(identifyDef)
	identifyDefFile.close()
	
	strHead = '#ifndef DISP_CTRL_TYPEDEF_H_\n#define DISP_CTRL_TYPEDEF_H_\n\n#include <string>\n\n'
	strTail = '\n#endif'
	dataStructDef = strHead+'\n'+GenerateStruct(root)+strTail
	dataStructDef = FormatCode(dataStructDef)
	print(dataStructDef, end='\n\n')
	dataDefFile = open(outputPath+'DispCtrlTypedef.h', 'w')
	dataDefFile.write(dataStructDef)
	dataDefFile.close()
	
	strHead = '#ifndef DISP_CTRL_INTERFACE_H_\n#define DISP_CTRL_INTERFACE_H_\n\n#include <string>\n\n#include "GenerateCode.h"\n#include "DispCtrlTypedef.h"\n\n'
	strTail = '\n#endif'
	msgInterfaceDef = strHead+GenerateMsgInterface(root)+strTail
	msgInterfaceDef = FormatCode(msgInterfaceDef)
	print(msgInterfaceDef, end='\n\n')
	msgInterfaceDefFile = open(outputPath+'DispCtrlInterface.h', 'w')
	msgInterfaceDefFile.write(msgInterfaceDef)
	msgInterfaceDefFile.close()
	
	strHead = '#ifndef DISP_CTRL_TYPESERIALIZE_H_\n#define DISP_CTRL_TYPESERIALIZE_H_\n\n#include <string>\n\n#include "BaseTypeSerialize.h"\n#include "DispCtrlTypedef.h"\n\n'
	strTail = '\n#endif'
	structCodecFuncDef = strHead+GenerateStructCodecFunc(root)+strTail
	structCodecFuncDef = FormatCode(structCodecFuncDef)
	print(structCodecFuncDef, end='\n\n')
	structCodecFuncDefFile = open(outputPath+'DispCtrlTypedefSerialize.h', 'w')
	structCodecFuncDefFile.write(structCodecFuncDef)
	structCodecFuncDefFile.close()
	
	strHead = SET_OPERATE_DECAL_HEAD.replace('ClassName', CLASS_NAME)
	strTail = SET_OPERATE_DECAL_TAIL.replace('ClassName', CLASS_NAME)
	msgProcFuncDeclaration = strHead+GenerateMsgProcFuncDeclaration(root)+strTail
	msgProcFuncDeclaration = FormatCode(msgProcFuncDeclaration)
	print(msgProcFuncDeclaration, end='\n\n')
	outputPath = "..\\hmi\\Application\\src\\"
	msgProcFuncDeclaFile = open(outputPath+MSG_PROC_FILE_NAME+'.h', 'w')
	msgProcFuncDeclaFile.write(msgProcFuncDeclaration)
	msgProcFuncDeclaFile.close()
	
	strHead = SET_OPERATE_DEF_HEAD.replace('ClassName', CLASS_NAME)
	strTail = ''
	msgProcFuncDef = strHead+GenerateMsgProcFuncDefinition(root, '')+strTail
	msgProcFuncDef = FormatCode(msgProcFuncDef)
	print(msgProcFuncDef, end='\n\n')
	msgProcFuncDefFile = open(outputPath+MSG_PROC_FILE_NAME+'.cpp', 'w')
	msgProcFuncDefFile.write(msgProcFuncDef)
	msgProcFuncDefFile.close()