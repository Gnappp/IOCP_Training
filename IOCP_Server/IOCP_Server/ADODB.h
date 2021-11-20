#pragma once

#include <vector>

#pragma warning(push)
#pragma warning(disable:4146)
#import "c:\Program Files (x86)\Common Files\System\ADO\msado15.dll" no_namespace rename("EOF", "EndOfFile")
#pragma warning(pop)

using namespace std;

class CAdodb
{
public:
	CAdodb();
	~CAdodb();

	void Initialize();
	bool Connect(const char* provider, ExecuteOptionEnum emOption = adOptionUnspecified);
	bool ExcuteQry(const char* qry);
	bool ExcuteQry(_bstr_t& btQry);

private:
	_ConnectionPtr connectionPtr;
	_RecordsetPtr recordsetPtr;

	_bstr_t btProvider;
	long colSize;
	long rowSize;

	vector<_variant_t>* vecDatas; // _variant_t 당 컬럼하나

	template <typename t>
	bool GetData(int col, int row, t* data, int size = 0)
	{
		try
		{
			vector<_variant_t>::iterator iter = vecDatas[col].begin();
			if (row >= vecDatas[col].size())
			{
				return false;
			}
			_variant_t vtData = *(iter + row);

			switch (vtData.vt)
			{
			case VT_NULL:
			case VT_EMPTY:
				if (isNull != NULL)
				{
					*isNull = true;
				}
				return false;
				break;
			default:
				*data = vtData.lVal;
				if (isNull != NULL)
				{
					*isNull = false;
				}
				break;
			}
		}
		catch (_com_error& e)
		{
			cout << "GetData Error" << e.ErrorMessage() << endl;
			return false;
		}
		return true;
	}	
};