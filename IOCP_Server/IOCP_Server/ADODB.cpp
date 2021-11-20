#include "ADODB.h"

#include <iostream>

CAdodb::CAdodb()
{
	connectionPtr = NULL;
	recordsetPtr = NULL;
	vecDatas = NULL;
	colSize = 0;
	rowSize = 0;
	Initialize();
}

CAdodb::~CAdodb()
{
	if (vecDatas != NULL)
	{
		delete[] vecDatas;
		vecDatas = NULL;
	}
}

void CAdodb::Initialize()
{
	connectionPtr = NULL;
	recordsetPtr = NULL;

	colSize = 0;
	rowSize = 0;

	if (vecDatas != NULL)
	{
		delete[] vecDatas;
		vecDatas = NULL;
	}
}

bool CAdodb::Connect(const char* provider, ExecuteOptionEnum emOption)
{
	if (connectionPtr != NULL)
		return false;

	try
	{
		connectionPtr.CreateInstance("ADODB.Connection");
		if (connectionPtr == NULL)
			cout << "connectionPtr is NULL" << endl;
		recordsetPtr.CreateInstance(__uuidof(Recordset));
		if (recordsetPtr == NULL)
			cout << "recordsetPtr is NULL" << endl;

		btProvider = (_bstr_t)provider;
		cout << btProvider << endl;
		connectionPtr->Open(btProvider, _bstr_t(""), _bstr_t(""), emOption);
	}
	catch (_com_error& e)
	{
		if (NULL == provider)
			cout << "COFdbConnect Error" << endl << "Query = " << provider << endl << "Code = " << e.Description() << endl << "Code meaning = " << e.ErrorMessage() << endl;
		else
			cout << "COFdbConnect Error" << endl << "Query = " << provider << endl << "Code = " << e.Description() << endl << "Code meaning = " << e.ErrorMessage() << endl;

		return false;
	}
	return true;
}

bool CAdodb::ExcuteQry(const char* qry)
{
	_bstr_t btQry(qry);
	return this->ExcuteQry(btQry);
}

bool CAdodb::ExcuteQry(_bstr_t& btQry)
{
	rowSize = 0;
	if (connectionPtr == NULL)
		return false;
	if (connectionPtr->GetState() != adStateOpen)
		return false;

	try
	{
		recordsetPtr = connectionPtr->Execute(btQry, NULL, adOptionUnspecified);

		colSize = recordsetPtr->GetFields()->GetCount();
		if (colSize == 0)
			return true;
		if (vecDatas != NULL)
		{
			delete[] vecDatas;
			vecDatas = NULL;
		}
		vecDatas = new vector<_variant_t>[colSize];
		while (recordsetPtr->GetEndOfFile())
		{
			for (int i = 0; i < colSize; i++)
			{
				FieldPtr fieldPtr = recordsetPtr->Fields->GetItem(i);
				_bstr_t name = fieldPtr->GetName();
				_variant_t vtQryData = recordsetPtr->GetCollect(name);

				vecDatas[i].push_back(vtQryData);
			}
			recordsetPtr->MoveNext();
			++rowSize;
		}
	}
	catch (_com_error& e)
	{
		for (int i = 0; i < connectionPtr->Errors->GetCount(); i++)
		{
			ErrorPtr err = connectionPtr->Errors->GetItem(i);
			if (err->SQLState == _bstr_t("08S01"))
			{
				cout << "Disconnect - " << btQry << endl;
				break;
			}
			err->NativeError;
		}
		return false;
	}
	return true;
}