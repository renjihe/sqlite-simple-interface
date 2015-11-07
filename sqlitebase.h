#ifndef _SQLITE_DATABASE_H_
#define _SQLITE_DATABASE_H_

#include "sqlite3.h"
#include <map>

enum SQLITE_DATATYPE  
{  
    SQLITE_DATATYPE_INTEGER = SQLITE_INTEGER,  
    SQLITE_DATATYPE_FLOAT = SQLITE_FLOAT,  
    SQLITE_DATATYPE_TEXT = SQLITE_TEXT,  
    SQLITE_DATATYPE_BLOB = SQLITE_BLOB,  
    SQLITE_DATATYPE_NULL = SQLITE_NULL,  
};  
  
struct ColInfo 
{
    SQLITE_DATATYPE nType;
    void *pData;
    int nLen;
};

class SQLiteBase 
{  
public:  
    SQLiteBase(void);  
    ~SQLiteBase(void);  
public:   
    bool Open(const char * sDbFlie);  
    void Close();
    bool isOpen() {
        return _bOpen;
    }
  
    bool BeginSQL();
    bool EndSQL();  
    
    bool Prepare(const char * sSQL);
    bool ExcuteNonQuery(const char * sSQL);
    bool ExeSQL(const char * sSQL);
    bool Fetch();
    
    void CloseCursor();
    
    bool BindCol(SQLITE_DATATYPE nType, void* pOutBuffer, unsigned nOutBufferLen, int nColIndex = -1);
    bool BindBinParam(SQLITE_DATATYPE nType, void* pInBuffer, unsigned nInSize, int nParamIndex = -1);
   
    inline int GetColCount()
    {
        return sqlite3_column_count(_pStmt);
    };
        
    const char *GetName(int nCol)  
    {  
        return (const char *)sqlite3_column_name(_pStmt, nCol);  
    }  

    SQLITE_DATATYPE GetDataType(int nCol)  
    {  
        return (SQLITE_DATATYPE)sqlite3_column_type(_pStmt, nCol);  
    }  
    
    bool BeginTransaction();  
    bool CommitTransaction();  
    bool RollbackTransaction();  

    const char * GetLastErrorMsg();  
private:  
    sqlite3 *_pDB;
    sqlite3_stmt* _pStmt;
    
    int _nPreBindColIndex, _nPreBindParameter;
    std::map<unsigned, ColInfo> _mCol;
    
    bool _bOpen;
}; 

extern SQLiteBase g_SQLiteBase;

#endif
