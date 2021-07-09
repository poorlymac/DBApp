#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>

#include </usr/local/Cellar/mysql-client/8.0.25/include/mysql/mysql.h>
#include "webview.h"
#include "DBApp.h"
typedef unsigned char uchar;

using namespace std;

webview::webview w(true, nullptr);

// https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c

static std::string base64_encode(const std::string &in) {
    std::string out;
    int val = 0, valb = -6;
    for (uchar c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val>>valb)&0x3F]);
            valb -= 6;
        }
    }
    if (valb>-6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}

static std::string base64_decode(const std::string &in) {
    std::string out;
    std::vector<int> T(256,-1);
    for (int i=0; i<64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
    int val=0, valb=-8;
    for (uchar c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val>>valb)&0xFF));
            valb -= 8;
        }
    }
    return out;
}

string encryptDecrypt(string toEncrypt) {
    string key = "Wynona'sGotHerselfABigBrownBeaverAndSheShowsItOffToAllHerFriends";
    string output = toEncrypt;
    for (string::size_type i = 0; i < toEncrypt.size(); i++) {
        output[i] = toEncrypt[i] ^ key[i];
    }
    return output;
}

std::string ReplaceString(std::string subject, const std::string& search, const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2)
{
    hex1 = c / 16;
    hex2 = c % 16;
    hex1 += hex1 <= 9 ? '0' : 'a' - 10;
    hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}

string urlencode(string s) {
    const char *str = s.c_str();
    vector<char> v(s.size());
    v.clear();
    for (size_t i = 0, l = s.size(); i < l; i++) {
        char c = str[i];
        if ((c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
            c == '*' || c == '\'' || c == '(' || c == ')')
        {
            v.push_back(c);
        } else if (c == ' ') {
            v.push_back('+');
        } else {
            v.push_back('%');
            unsigned char d1, d2;
            hexchar(c, d1, d2);
            v.push_back(d1);
            v.push_back(d2);
        }
    }
    return string(v.cbegin(), v.cend());
}

 #ifdef __APPLE__
    // Helpers to avoid too much typing
    id operator"" _cls(const char *s, std::size_t) { return (id)objc_getClass(s); }
    SEL operator"" _sel(const char *s, std::size_t) { return sel_registerName(s); }
    id operator"" _str(const char *s, std::size_t) {
        return ((id(*)(id, SEL, const char *))objc_msgSend)("NSString"_cls, "stringWithUTF8String:"_sel, s);
    }
    static id create_menu_item(id title, const char *action, const char *key) {
            id item = ((id(*)(id, SEL))objc_msgSend)((id)"NSMenuItem"_cls, "alloc"_sel);
            ((id(*)(id, SEL, id, SEL, id))objc_msgSend)(
                item,
                "initWithTitle:action:keyEquivalent:"_sel,
                title,
                sel_registerName(action),
                ((id(*)(id, SEL, const char *))objc_msgSend)("NSString"_cls, "stringWithUTF8String:"_sel, key)
            );
            ((id(*)(id, SEL))objc_msgSend)(item, "autorelease"_sel);
            return item;
    }
#endif

std::string getEnvVar(std::string const& key) {
    char const* val = getenv(key.c_str()); 
    return val == NULL ? std::string() : std::string(val);
}

std::string escape_json(const std::string &s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        if (*c == '"' || *c == '\\' || ('\x00' <= *c && *c <= '\x1f')) {
            o << "\\u"
              << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
        } else {
            o << *c;
        }
    }
    return o.str();
}

std::string escape_json1(const std::string &s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        switch (*c) {
        case '\x00': o << "\\u0000"; break;
        case '\x01': o << "\\u0001"; break;
        // ...
        case '\x0a': o << "\\n"; break;
        // ...
        case '\x1f': o << "\\u001f"; break;
        case '\x22': o << "\\\""; break;
        case '\x5c': o << "\\\\"; break;
        default: o << *c;
        }
    }
    return o.str();
}

std::string escape_json2(const std::string &s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        switch (*c) {
        case '"': o << "\\\""; break;
        case '\\': o << "\\\\"; break;
        case '\b': o << "\\b"; break;
        case '\f': o << "\\f"; break;
        case '\n': o << "\\n"; break;
        case '\r': o << "\\r"; break;
        case '\t': o << "\\t"; break;
        default:
            if ('\x00' <= *c && *c <= '\x1f') {
                o << "\\u"
                  << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
            } else {
                o << *c;
            }
        }
    }
    return o.str();
}

std::string escape_json3(const std::string &s) {
    std::string output;
    output.reserve(s.length());
    for (std::string::size_type i = 0; i < s.length(); ++i) {
        switch (s[i]) {
            case '"':
                output += "\\\"";
                break;
            case '/':
                output += "\\/";
                break;
            case '\b':
                output += "\\b";
                break;
            case '\f':
                output += "\\f";
                break;
            case '\n':
                output += "\\n";
                break;
            case '\r':
                output += "\\r";
                break;
            case '\t':
                output += "\\t";
                break;
            case '\\':
                output += "\\\\";
                break;
            default:
                output += s[i];
                break;
        }
    }
    return output;
}

string mysql_type(int type) {
    string t = "unknown";
    switch (type) {
        case 1:
            t = "decimal";
            break;
        case 2:
            t = "tiny";
            break;
        case 3:
            t = "short";
            break;
        case 4:
            t = "long";
            break;
        case 5:
            t = "float";
            break;
        case 6:
            t = "double";
            break;
        case 7:
            t = "null";
            break;
        case 8:
            t = "timestamp";
            break;
        case 9:
            t = "longlong";
            break;
        case 10:
            t = "int24";
            break;
        case 11:
            t = "date";
            break;
        case 12:
            t = "time";
            break;
        case 13:
            t = "datetime";
            break;
        case 14:
            t = "year";
            break;
        case 15:
            t = "newdate";
            break;
        case 16:
            t = "varchar";
            break;
        case 17:
            t = "bit";
            break;
        case 18:
            t = "timestamp2";
            break;
        case 19:
            t = "datetime2";
            break;
        case 20:
            t = "time2";
            break;
        case 21:
            t = "typed array";
            break;
        case 243:
            t = "invalid";
            break;
        case 244:
            t = "bool";
            break;
        case 245:
            t = "json";
            break;
        case 246:
            t = "new decimal";
            break;
        case 247:
            t = "enum";
            break;
        case 248:
            t = "set";
            break;
        case 249:
            t = "tiny blob";
            break;
        case 250:
            t = "medium blob";
            break;
        case 251:
            t = "long blob";
            break;
        case 252:
            t = "blob";
            break;
        case 253:
            t = "var string";
            break;
        case 254:
            t = "string";
            break;
        case 255:
            t = "geometry";
            break;
    }
    return t;
}

std::string cnf;
MYSQL_RES *result;
MYSQL_ROW row;
MYSQL_FIELD *field;
MYSQL *connection, mysql;
unsigned int conn_timeout = 5;
string server;
string login;

string rpath = ".";

std::string loadfile(std::string filepath) {
    stringstream buffer;
    ifstream fs (filepath.c_str());
    if (fs.is_open()) {
        string line;
        stringstream buffer;
        while ( getline (fs, line) ) {
            buffer << line;
        }
        fs.close();
        return buffer.str();
    } else {
        return "";
    }
}

void signalHandler(int signum) {
    if (connection != NULL) {
        mysql_close(connection);
        exit(signum);
    }
}
void functionExit(void) {
    signalHandler(0);
}

string getDatabaseInformation() {
    // Get the list of databases
    int state = mysql_query(connection, "SELECT version() VERSION, CATALOG_NAME, SCHEMA_NAME, DEFAULT_CHARACTER_SET_NAME, DEFAULT_COLLATION_NAME, COALESCE(ROUND(SUM(data_length + index_length) / 1024 / 1024, 2), 0) AS MB, SUM(CASE WHEN table_type = 'BASE TABLE' THEN 1 ELSE 0 END) TABLES, SUM(CASE WHEN table_type = 'VIEW' THEN 1 ELSE 0 END) VIEWS FROM information_schema.schemata s LEFT OUTER JOIN information_schema.tables t ON t.table_schema = s.schema_name GROUP BY CATALOG_NAME, SCHEMA_NAME, DEFAULT_CHARACTER_SET_NAME, DEFAULT_COLLATION_NAME ORDER BY 1, 2, 3");
    if (state !=0) {
        return "{\"message\":\"Error getting schema information: " + std::string(mysql_error(connection)) + "\", \"result\":false}";
    }
    result = mysql_store_result(connection);
    stringstream datastream;
    double mb_max = 0.0;
    double mb_tot = 0.0;
    int tables_tot = 0;
    int views_tot = 0;
    string version = "";
    string data = "";
    if (mysql_num_rows(result) > 0) {
        while ( ( row = mysql_fetch_row(result)) != NULL ) {
            version          = row[0];
            string catalog   = row[1];
            string schema    = row[2];
            string charset   = row[3];
            string collation = row[4];
            double mb        = atof(row[5]);
            int tables       = atoi(row[6]);
            int views        = atoi(row[7]);
            if(mb > mb_max) {
                mb_max = mb;
            }
            mb_tot += mb;
            tables_tot += tables;
            views_tot += views;
            datastream << "{\"catalog\":\"" + catalog + "\",\"schema\":\"" + schema + "\",\"charset\":\"" + charset + "\",\"collation\":\"" + collation + "\",\"mb\":\"" << mb << "\",\"tables\":\"" << tables << "\",\"views\":\"" << views << "\"}," << endl;
        }
        data = datastream.str().substr(0, datastream.str().size() - 1); // truncate the last comma
    }
    mysql_free_result(result);
    return "{\"message\":\"Logged in to " + server + " as " + login + "\", \"result\":true,\"version\":\"" + version + "\",\"data\":[" + data + "], \"tables_tot\":\"" + to_string(tables_tot) + "\", \"views_tot\":\"" + to_string(views_tot) + "\", \"mb_tot\":\"" + to_string(mb_tot) + "\", \"mb_max\":\"" + to_string(mb_max) + "\"}";
}

string getDatabaseTableInformation(string catalog, string schema) {
    // Get the list of tables
    // TODO protect from injection
    string sql = "SELECT t.table_name, t.table_type, t.engine, t.table_rows, COALESCE(ROUND(t.data_length / 1024 / 1024, 2), 0), COALESCE(ROUND(t.index_length / 1024 / 1024, 2), 0), t.table_comment, GROUP_CONCAT(CASE WHEN c.column_name LIKE '% %' THEN CONCAT('`', c.column_name, '`') ELSE c.column_name END ORDER BY c.ordinal_position SEPARATOR ', ') columns FROM information_schema.tables t LEFT OUTER JOIN information_schema.columns c ON t.table_schema = c.table_schema AND t.table_name = c.table_name WHERE t.table_schema = '" + schema + "' AND t.table_catalog = '" + catalog + "' GROUP BY 1, 2, 3, 4, 5, 6, 7 ORDER BY t.table_type, t.table_name";
    int state = mysql_query(connection, sql.c_str());
    if (state !=0) {
        return "{\"message\":\"Error getting table information: " + std::string(mysql_error(connection)) + "\", \"result\":false}";
    }
    result = mysql_store_result(connection);
    stringstream datastream;
    unsigned long long data_length_tot = 0;
    unsigned long long index_length_tot = 0;
    unsigned long long table_rows_tot = 0;
    unsigned long long rowcount = 0;
    if (mysql_num_rows(result) > 0) {
        while ( ( row = mysql_fetch_row(result)) != NULL ) {
            if (rowcount != 0) {
                datastream << ",";
            }
            rowcount++;
            string table_name = row[0]==NULL?"":row[0];
            string table_type = row[1]==NULL?"":row[1];
            string engine     = row[2]==NULL?"":row[2];
            unsigned long long table_rows   = atoll(row[3]==NULL?"":row[3]);
            unsigned long long data_length  = atoll(row[4]==NULL?"":row[4]);
            unsigned long long index_length = atoll(row[5]==NULL?"":row[5]);
            string table_comment = row[6]==NULL?"":row[6];
            string columns = row[7]==NULL?"":row[7];
            data_length_tot += data_length;
            index_length_tot += index_length;
            table_rows_tot += table_rows;
            datastream << "{\"table_name\":\"" + escape_json(table_name) + "\",\"table_type\":\"" + escape_json(table_type) + "\",\"engine\":\"" + escape_json(engine) + "\",\"table_rows\":\"" << table_rows << "\",\"data_length\":\"" << data_length << "\",\"index_length\":\"" << index_length << "\",\"table_comment\":\"" + escape_json(table_comment) + "\",\"columns\":\"" + escape_json(columns) + "\"}";
        }
    }
    mysql_free_result(result);
    return "{\"message\":\"Retrieved table information for catalog " + escape_json(catalog) + " schema " + escape_json(schema) + "\", \"result\":true,\"data\":[" + datastream.str() + "], \"data_length_tot\":\"" + to_string(data_length_tot) + "\", \"index_length_tot\":\"" + to_string(index_length_tot) + "\", \"table_rows_tot\":\"" + to_string(table_rows_tot) + "\"}";
}

string runSQL(string catalog, string schema, string sql) {
    if (mysql_select_db(connection, schema.c_str())) {
        return "{\"message\":\"Failed to set database: " + escape_json(mysql_error(connection)) + "\",\"sql\":\"" + escape_json(sql) + "\", \"result\":false}";
    }
    if (mysql_real_query(connection, sql.c_str(), strlen(sql.c_str()))) {
        return "{\"message\":\"Error running query: " + escape_json(mysql_error(connection)) + "\",\"sql\":\"" + escape_json(sql) + "\", \"result\":false}";
    }
    MYSQL_RES *runresult = mysql_use_result(connection);
    stringstream htmltable;
    stringstream columnstream;
    stringstream columnnamestream;
    char* p;
    unsigned long long rowcount = 0;
    int num_fields = mysql_num_fields(runresult);
    htmltable << "<table class='sortable' style='font-size:75%'>";
    while ((row = mysql_fetch_row(runresult)) != NULL) {
        if (rowcount == 0) {
            htmltable << "<thead><tr>";
            for(int i = 0; i < num_fields; i++) {
                if (i != 0) {
                    columnstream << ",";
                    columnnamestream << ",";
                }
                field = mysql_fetch_field_direct(runresult, i);
                columnstream << "{\"name\":\"" << escape_json(field->name) << "\",\"type\":\"" << mysql_type(field->type) << "\"}" << endl;
                columnnamestream << "\"" << escape_json(field->name) << "\":{\"type\":\"" << mysql_type(field->type) << "\", \"position\":" << i << "}" << endl;
                htmltable << "<th>" << field->name << "</th>";
            }
            htmltable << "</tr></thead><tbody>";
        }
        rowcount++;
        htmltable << "<tr>";
        for(int i = 0; i < num_fields; i++) {
            field = mysql_fetch_field_direct(runresult, i);
            if (row[i]==NULL) {
                htmltable << "<td>null</td>";
            } else {
                auto value = row[i];
                if(strlen(value) == 0) {
                    htmltable << "<td></td>";
                } else {
                    // Display numbers raw
                    long converted = strtol(value, &p, 10);
                    if (*p) {
                        htmltable << "<td>" << value << "</td>";
                    } else {
                        htmltable << "<td style=\"text-align: right;\">" << value << "</td>";
                    }
                }
            }
        }
        htmltable << "</tr>";
        if (rowcount % 10000 == 0) {
            w.eval("document.getElementById('message').innerHTML = 'Retrieved " + to_string(rowcount) + " rows ...';");
        }
    }
    htmltable << "</tbody></table>";
    w.eval("document.getElementById('message').innerHTML = 'Retrieved " + to_string(rowcount) + " rows, HTML is tabulating <blink>...</blink>'");
    mysql_free_result(runresult);
    return "{\"message\":\"Retrieved " + to_string(rowcount) + " rows\", \"catalog\":\"" + escape_json(catalog) + "\", \"schema\":\"" + escape_json(schema) + "\",\"sql\":\"" + escape_json(sql) + "\", \"result\":true,\"columns\":{\"column\":[" + columnstream.str() + "], \"columnname\":{" + columnnamestream.str() + "}},\"rowcount\":\"" + to_string(rowcount) + "\",\"num_fields\":\"" + to_string(num_fields) + "\",\"htmltable\":\"" + base64_encode(htmltable.str()) + "\"}";
}

#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine,int nCmdShow) {
#else
int main() {
#endif
    signal(SIGINT, signalHandler);
    atexit (functionExit);
    cnf = getEnvVar("HOME") +  "/.DBApp.cnf";
    #ifdef __APPLE__

        // get the OSX resource path
        id bundle = ((id(*)(id, SEL))objc_msgSend)((id)"NSBundle"_cls, "mainBundle"_sel);
        id brpath = ((id(*)(id, SEL))objc_msgSend)(bundle, "resourcePath"_sel);
        rpath = ((const char *(*)(id, SEL))objc_msgSend)(brpath, "UTF8String"_sel);

        // Thanks goes to https://github.com/lukevers/webview/blob/edit-menu/webview.h fork for all of this section

        /*** Create menubar ***/
        id menubar = ((id(*)(id, SEL))objc_msgSend)((id)"NSMenu"_cls, "alloc"_sel);
        ((id(*)(id, SEL, id))objc_msgSend)(menubar, "initWithTitle:"_sel, ""_str);
        ((id(*)(id, SEL))objc_msgSend)(menubar, "autorelease"_sel);

        id appName = ((id(*)(id, SEL))objc_msgSend)(
        ((id(*)(id, SEL))objc_msgSend)((id)"NSProcessInfo"_cls, "processInfo"_sel),
        "processName"_sel
        );

        /*** Main application menu ***/
        id appMenuItem = ((id(*)(id, SEL))objc_msgSend)((id)"NSMenuItem"_cls, "alloc"_sel);
        ((id(*)(id, SEL, id, id, id))objc_msgSend)(
            appMenuItem,
            "initWithTitle:action:keyEquivalent:"_sel,
            appName,
            NULL,
            ""_str
        );

        id appMenu = ((id(*)(id, SEL))objc_msgSend)((id)"NSMenu"_cls, "alloc"_sel);
        ((id(*)(id, SEL, id))objc_msgSend)(appMenu, "initWithTitle:"_sel, appName);
        ((id(*)(id, SEL))objc_msgSend)(appMenu, "autorelease"_sel);

        ((id(*)(id, SEL, id))objc_msgSend)(appMenuItem, "setSubmenu:"_sel, appMenu);
        ((id(*)(id, SEL, id))objc_msgSend)(menubar,"addItem:"_sel, appMenuItem);

        id title = ((id(*)(id, SEL, id))objc_msgSend)("Hide "_str, "stringByAppendingString:"_sel, appName);
        id item = create_menu_item(title, "hide:", "h");
        ((id(*)(id, SEL, id))objc_msgSend)(appMenu, "addItem:"_sel, item);

        title = ((id(*)(id, SEL, id))objc_msgSend)(
            "Quit "_str,
            "stringByAppendingString:"_sel,
            appName
        );
        item = create_menu_item(title, "terminate:", "q");
        ((id(*)(id, SEL, id))objc_msgSend)(appMenu, "addItem:"_sel, item);

        /*** Edit menu ***/
        id editMenuItem = ((id(*)(id, SEL))objc_msgSend)((id)"NSMenuItem"_cls, "alloc"_sel);
        ((id(*)(id, SEL, id, id, id))objc_msgSend)(
            editMenuItem,
            "initWithTitle:action:keyEquivalent:"_sel,
            "Edit"_str,
            NULL,
            ""_str
        );

        id editMenu = ((id(*)(id, SEL))objc_msgSend)((id)"NSMenu"_cls, "alloc"_sel);
        ((id(*)(id, SEL, id))objc_msgSend)(editMenu, "initWithTitle:"_sel, "Edit"_str);
        ((id(*)(id, SEL))objc_msgSend)(editMenu, "autorelease"_sel);

        ((id(*)(id, SEL, id))objc_msgSend)(editMenuItem, "setSubmenu:"_sel, editMenu);
        ((id(*)(id, SEL, id))objc_msgSend)(menubar, "addItem:"_sel, editMenuItem);

        item = create_menu_item("Cut"_str, "cut:", "x");
        ((id(*)(id, SEL, id))objc_msgSend)(editMenu, "addItem:"_sel, item);

        item = create_menu_item("Copy"_str, "copy:", "c");
        ((id(*)(id, SEL, id))objc_msgSend)(editMenu, "addItem:"_sel, item);

        item = create_menu_item("Paste"_str, "paste:", "v");
        ((id(*)(id, SEL, id))objc_msgSend)(editMenu, "addItem:"_sel, item);

        item = create_menu_item("Select All"_str, "selectAll:", "a");
        ((id(*)(id, SEL, id))objc_msgSend)(editMenu, "addItem:"_sel, item);
        
        ((id(*)(id, SEL, id))objc_msgSend)(
            ((id(*)(id, SEL))objc_msgSend)((id)"NSApplication"_cls, "sharedApplication"_sel),
            "setMainMenu:"_sel,
            menubar
        );
    #endif

    mysql_init(&mysql);
    mysql_options(&mysql, MYSQL_OPT_CONNECT_TIMEOUT, &conn_timeout);
    w.set_title("DBApp");
    w.set_size(1500, 1000, WEBVIEW_HINT_NONE);
    w.set_size(180, 120, WEBVIEW_HINT_MIN);

    // login function
    w.bind("wvlogin", [](std::string s) -> std::string {
        auto db = webview::json_parse(s, "", 0);
        // std::cout << s << std::endl;
        server   = webview::json_parse(db, "server", 0);
        login    = webview::json_parse(db, "login", 1);
        auto password = webview::json_parse(db, "password", 2);

        // Write config to file
        ofstream myconf;
        myconf.open(cnf);
        myconf << "{\"server\":\"" +  escape_json(server) + "\", \"login\":\"" + escape_json(login) + "\", \"password\":\"" + escape_json(webview::url_encode(encryptDecrypt(password))) + "\"}";
        myconf.close();

        // TODO MySQL Login
        connection = mysql_real_connect(&mysql, server.c_str(), login.c_str(), password.c_str(), "mysql", 0, 0, 0);
        if (connection == NULL) {
            return "{\"message\":\"Error logging into " + escape_json(server) + " as " + escape_json(login) + ": " + escape_json(mysql_error(&mysql)) + "\", \"result\":false}";
        } else {
            return getDatabaseInformation();
        }
    });

    // logout function
    w.bind("wvlogout", [](std::string s) -> std::string {
        s = "No Connection closed";
        if (connection != NULL) {
            s = "Closed Database Connection";
            mysql_close(connection);
            connection = NULL;
        }
        return "{\"message\":\"" + s + "\"}";
    });

    // tables function
    w.bind("wvtables", [](std::string s) -> std::string {
        auto catalog = webview::json_parse(s, "", 0);
        auto schema = webview::json_parse(s, "", 1);
        return getDatabaseTableInformation(catalog, schema);
    });

    // tables function
    w.bind("wvschemas", [](std::string s) -> std::string {
        webview::json_parse(s, "", 0);
        return getDatabaseInformation();
    });

    // Run SQL function
    w.bind("wvsql", [](std::string s) -> std::string {
        auto catalog = webview::json_parse(s, "", 0);
        auto schema = webview::json_parse(s, "", 1);
        auto sql = webview::json_parse(s, "", 2);
        return runSQL(catalog, schema, sql);
    });

    // Construct and load main HTML
    std::string s(DBApp_html, DBApp_html + DBApp_html_len);
    std::string page = "data:text/html,";
    page.append(urlencode(s));
    // Add in any styles to the main page
    page.append(urlencode("<style>"));
    page.append(urlencode(loadfile(rpath + "/DBApp.css")));
    page.append(urlencode(loadfile(rpath + "/sortable.css")));
    page.append(urlencode(loadfile(rpath + "/TextareaDecorator.css")));
    page.append(urlencode("</style>"));
    w.navigate(page.c_str());

    string cnfstr = loadfile(cnf);
    if (cnfstr != "") {
        server   = webview::json_parse(cnfstr, "server", 0);
        login    = webview::json_parse(cnfstr, "login", 0);
        auto password = encryptDecrypt(webview::url_decode(webview::json_parse(cnfstr, "password", 0)));
        w.init("var server = \"" + escape_json(server) + "\";");
        w.init("var login = \"" + escape_json(login) + "\";");
        w.init("var password = \"" + escape_json(password) + "\";");
    } else {
        w.init("var server = '';");
        w.init("var login = '';");
        w.init("var password = '';");
    }
    
    // Load other JS support files
    w.init(loadfile(rpath + "/DBApp.js"));
    w.init(loadfile(rpath + "/sortable.js"));
    w.init(loadfile(rpath + "/Keybinder.js"));
    w.init(loadfile(rpath + "/Parser.js"));
    w.init(loadfile(rpath + "/SelectHelper.js"));
    w.init(loadfile(rpath + "/TextareaDecorator.js"));

    w.run();
    mysql_close(connection);
    return 0;
}
