# sqlite-simple-interface
a simple interface for sqlite library

sample

if (!g_SQLiteBase.IsOpen()) {
    g_SQLiteBase.Open("/exec_dir/main.db3")
}
if (g_SQLiteBase.BeginSQL())
{
    int id;
    char name[255];
    if (g_SQLiteBase.ExeSQL("select id, name from test;"))
    {
        int id;
        char name[255];
        g_SQLiteBase.BindCol(SQLITE_DATATYPE_INTEGER, &id, sizeof(id), 0);
        g_SQLiteBase.BindCol(SQLITE_DATATYPE_TEXT, &name, sizeof(name));
        while(g_SQLiteBase.Fetch()) {
            printf("id:%d, name:%s", id, name);
        }
    }
    if (!g_SQLiteBase.ExcuteNonQuery("insert into test (id, name) values (95, 'hello world')")) 
    {
        printf("ExcuteNonQuery faild!");
    }
    if (g_SQLiteBase.ExeSQL("insert into test (id, name) values (96, ?)"))
    {
        _G.sqliteBindBinParam(SQLITE_DATATYPE_TEXT, name, 10, 0);
            
        if (g_SQLiteBase.Fetch()) {
            printf("ExeSQL faild!");
        }
    }
    g_SQLiteBase.EndSQL();
}
