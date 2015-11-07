#include "sqlitebase.h"

SQLiteBase g_SQLiteBase;

SQLiteBase::SQLiteBase(void):  
_pDB(NULL), _pStmt(NULL)
{  
    _bOpen = false;
    _nPreBindColIndex = _nPreBindParameter = -1;
}  
  
SQLiteBase::~SQLiteBase(void)  
{  
    Close();  
}  
  
bool SQLiteBase::Open(const char * sDbFlie)  
{  
    if (_bOpen)
        return true;
        
    if (sDbFlie == NULL) {  
        return false;  
    } 

    if (sqlite3_open(sDbFlie, &_pDB) != SQLITE_OK) {  
        return false;  
    }  
    
    _bOpen = true;
    
    return true;  
}  
  
void SQLiteBase::Close()  
{  
    if(_pDB) {  
        sqlite3_close(_pDB);
        _pDB = NULL;
        _bOpen = false;
    }  
}  
  
bool SQLiteBase::ExcuteNonQuery(const char * sSQL)  
{  
    if(sSQL == NULL) {  
        return false;  
    }  

    sqlite3_stmt* stmt;    
    if(sqlite3_prepare_v2(_pDB, sSQL, -1, &stmt, NULL) != SQLITE_OK)  
    {   
        return false;    
    }    
    sqlite3_step(stmt);  

    return (sqlite3_finalize(stmt) == SQLITE_OK) ? true : false ;  
}  

bool SQLiteBase::BeginSQL()
{
    _nPreBindColIndex = _nPreBindParameter = -1;
    _mCol.clear();
    return true;
}

bool SQLiteBase::EndSQL()
{
    CloseCursor();
    return true;
}

bool SQLiteBase::Prepare(const char * sSQL)
{
    if(sqlite3_prepare_v2(_pDB, sSQL, -1, &_pStmt, NULL) != SQLITE_OK) {   
        return false;    
    }
    return true;  
}
  
bool SQLiteBase::ExeSQL(const char * sSQL)  
{
    return Prepare(sSQL);  
}

bool SQLiteBase::Fetch()
{
    if(_pStmt == NULL) {  
        return false;  
    } 
    
    if(sqlite3_step(_pStmt) != SQLITE_ROW) {  
        return false;  
    }  
    
    std::map<unsigned, ColInfo>::iterator it = _mCol.begin();
    for (; it != _mCol.end(); ++it) {
        switch(it->second.nType)
        {
        case SQLITE_DATATYPE_INTEGER:
            *(int *)it->second.pData = sqlite3_column_int(_pStmt, it->first - 1);
            break;
        case SQLITE_DATATYPE_FLOAT:
            *(double *)it->second.pData = sqlite3_column_double(_pStmt, it->first - 1);
            break;
        case SQLITE_DATATYPE_TEXT:
            {
                int nLen = sqlite3_column_bytes(_pStmt, it->first - 1);
                if (it->second.nLen <= nLen + 1) {
                    nLen = it->second.nLen - 1;
                }
                if (nLen > 0) {
                    memset((char *)it->second.pData, 0, nLen + 1);
                    strncpy((char *)it->second.pData, (const char *)sqlite3_column_text(_pStmt, it->first - 1), nLen);
                    ((char *)it->second.pData)[nLen + 1] = '\0';
                }
                else if (it->second.nLen > 0){
                    ((char *)it->second.pData)[0] = '\0';
                } 
                
            }
            break;
        case SQLITE_DATATYPE_BLOB:
            {
                int nLen = sqlite3_column_bytes(_pStmt, it->first - 1);
                if (it->second.nLen <= nLen) {
                    nLen = it->second.nLen;
                }
                if (nLen > 0) {
                    memcpy((char *)it->second.pData, (const char *)sqlite3_column_blob(_pStmt, it->first - 1), nLen);  
                }
            }
            break;
        case SQLITE_DATATYPE_NULL:
        default:
            break;
        }
    }
    
    //return (sqlite3_reset(_pStmt) == SQLITE_OK) ? true : false ; 
    return true;
}

bool SQLiteBase::BindCol(SQLITE_DATATYPE nType, void* pOutBuffer, unsigned nOutBufferLen, int nColIndex)
{
    int col = nColIndex > 0 ? nColIndex : (nColIndex == 0 ? 1 : _nPreBindColIndex + 1);
    
    ColInfo info;
    info.nType = nType;
    info.pData = pOutBuffer;
    info.nLen = nOutBufferLen;
    _mCol[col] = info;
    
    _nPreBindColIndex = col;
    return true;
}
bool SQLiteBase::BindBinParam(SQLITE_DATATYPE nType, void* pInBuffer, unsigned nInSize, int nParamIndex)
{
    if(_pStmt == NULL) {
        return false;
    }
    
    int param_index = nParamIndex > 0 ? nParamIndex : (nParamIndex == 0 ? 1 : _nPreBindParameter + 1);
    
    bool ret = true;
    switch (nType)
    {
        case SQLITE_DATATYPE_INTEGER:
            if(sqlite3_bind_int(_pStmt, param_index, *(int*)pInBuffer) != SQLITE_OK) {  
                ret = false;  
            }
            break;
        case SQLITE_DATATYPE_FLOAT:
            if(sqlite3_bind_double(_pStmt, param_index, *(double*)pInBuffer) != SQLITE_OK) {  
                ret = false;  
            }
            break;
        case SQLITE_DATATYPE_TEXT:
            if(sqlite3_bind_text(_pStmt, param_index, (const char *)pInBuffer, -1, SQLITE_TRANSIENT) != SQLITE_OK) {  
                ret = false;  
            }
            break;
        case SQLITE_DATATYPE_BLOB:
            if(sqlite3_bind_blob(_pStmt, param_index, pInBuffer, nInSize, NULL) != SQLITE_OK) {  
                ret = false;  
            }
            break;
        case SQLITE_DATATYPE_NULL:
        default:
            break;
    }
    
    _nPreBindParameter = param_index;

    return ret;
}

void SQLiteBase::CloseCursor(){
    if (_pStmt) {
        sqlite3_finalize(_pStmt);
        _pStmt = NULL;
    }
}

bool SQLiteBase::BeginTransaction()  
{  
    char * errmsg = NULL;  
    if(sqlite3_exec(_pDB, "BEGIN TRANSACTION;", NULL, NULL, &errmsg) != SQLITE_OK)  
    {  
        return false;  
    }   
    return true;  
}  

bool SQLiteBase::CommitTransaction()  
{  
    char * errmsg = NULL;  
    if(sqlite3_exec(_pDB, "COMMIT TRANSACTION;;", NULL, NULL, &errmsg) != SQLITE_OK)  
    {  
        return false;  
    }   
    return true;  
}  

bool SQLiteBase::RollbackTransaction()  
{  
    char * errmsg = NULL;  
    if(sqlite3_exec(_pDB, "ROLLBACK  TRANSACTION;", NULL, NULL, &errmsg) != SQLITE_OK)  
    {  
        return false;  
    }   
    return true;  
}  

const char * SQLiteBase::GetLastErrorMsg()  
{     
    return sqlite3_errmsg(_pDB);  
}  
