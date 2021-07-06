/* WARNING: Do NOT forget to terminate your lines with ; otherwise it breaks in strange ways */
/* WARNING: use /* style commenting */

var schemaData;

function completeLogin() {
    document.getElementById("overlay").style.display = "none";
}

function showLogin() {
    document.getElementById("overlay").style.display = "block";
    document.getElementById("modal").style.display = "block";
}

var loggedinresponse = false;
function doLogin() {
    document.getElementById("modal").style.display = "none";
    var db = {
        server: document.getElementById('server').value,
        login: document.getElementById('login').value,
        password: document.getElementById('password').value
    };
    server = document.getElementById('server').value;
    login = document.getElementById('login').value;
    password = document.getElementById('password').value;
    document.getElementById('schemasinfo').innerHTML = "&nbsp;Fetching Schemas <blink>...</blink>";
    setTimeout(function(){
        if (!loggedinresponse) {
            document.getElementById('loginmessage').innerHTML = "<span style='color: red;'>Login Timed out</span>";
            showLogin();    
        }
        loggedinresponse = false;
    }, 10000);
    wvlogin(
        db
    ).then(
        function (res) {
            loggedinresponse = true;
            document.getElementById('loginButton').disabled = false;
            if (res.result) {
                document.getElementById('systemInfo').innerHTML = " mysql://" + login + "@" + server + "/ (" + res.version + ") <span class='database'></span> " + parseInt(res.data.length).toLocaleString() + " &#8611; &#128202; " + parseInt(res.tables_tot).toLocaleString() + " &#8611; &#128065;&#65039; " + parseInt(res.views_tot).toLocaleString() + " &#8611; " + Math.round(parseFloat(res.mb_tot)).toLocaleString() + " MB";
                document.getElementById('message').innerHTML = res.message;
                document.getElementById('message').style.color = "green";
                document.getElementById('logout').disabled = false;
                document.getElementById('runsql').disabled = false;                
                document.getElementById('clear').disabled = false;
                document.getElementById("overlay").style.display = "none";
                showSchemas(res);
            } else {
                document.getElementById('loginmessage').innerHTML = res.message;
                showLogin();
            }
        }
    );
}

function doLogout() {
    document.getElementById('logout').disabled = true;
    document.getElementById('runsql').disabled = true;
    document.getElementById('clear').disabled = true;
    document.getElementById('message').innerHTML = "";
    wvlogout().then(
        function(res) {
            document.getElementById('loginmessage').innerHTML = res.message;
            showLogin();
        }
    );
}

function refreshSchemas() {
    document.getElementById('schemasinfo').innerHTML = "&nbsp;Fetching Schemas <blink>...</blink>";
    var schemas = document.getElementById('schemas');
    if (schemas) {
        while( schemas.firstChild ){
            schemas.removeChild(schemas.firstChild);
        }    
    }
    wvschemas().then(
        function(res) {
            showSchemas(res);
        }
    );
}

function showSchemas(res) {
    var schemas = document.getElementById('schemas');
    schemaData = res.data;
    for (var i = 0; i < res.data.length; i = i + 1) {
        var schema = document.createElement("li");
        var a = document.createElement('a');
        var ap = document.createElement('a');
        var progress = document.createElement('progress');
        progress.style.width = "50px";
        progress.max = parseInt(res.mb_max);
        progress.value = parseInt(res.data[i].mb);
        var linkText = document.createTextNode(res.data[i].schema);
        ap.appendChild(progress);
        ap.href = "javascript:void(0)";
        ap.title = Math.round(parseFloat(res.data[i].mb)).toLocaleString() + " MB";
        schema.appendChild(ap);
        schema.appendChild(document.createTextNode(" "));
        a.appendChild(linkText);
        a.href = "javascript:void(0)";
        a.sn = i;
        a.onclick = function () {
            showSchemaInfo(this.sn);
            return false;
        };
        schema.appendChild(a);
        schemas.appendChild(schema);
    }
    document.getElementById('schemasinfo').innerHTML = "<button name=\"Refresh\" id=\"refreshButton\" onclick=\"refreshSchemas()\">Refresh Schemas</button>";
}

function showSchemaInfo(sn) {
    document.getElementById('catalog').value = schemaData[sn].catalog;
    document.getElementById('schema').value = schemaData[sn].schema;
    document.getElementById('tablelist').innerHTML = '';
    wvtables(
        schemaData[sn].catalog,
        schemaData[sn].schema
    ).then(
        function(tabinfo) {
            document.getElementById('schemainfo').innerHTML = "<button onclick=\"showSchemaInfo(" + sn + ")\" style=\"float: right;\">Refresh</button><b>" + schemaData[sn].schema + "</b>&nbsp;" + parseFloat(schemaData[sn].mb).toLocaleString() + "MB&nbsp;" + parseInt(schemaData[sn].tables).toLocaleString() + "&nbsp;Tables&nbsp;(" + schemaData[sn].catalog + "&nbsp;" + schemaData[sn].charset + "." + schemaData[sn].collation + ")";
            var txt = "<table class='sortable' style='font-size:75%'><thead><th>Table Name</th><th>~ Rows</th><th>~ Data / Index (MB)</th><th>~ Total (MB)</th><th>Comment</th></thead></tr><tbody>";
            for (var x in tabinfo.data) {
                var tr = parseFloat(tabinfo.data[x].table_rows);
                var dl = parseFloat(tabinfo.data[x].data_length);
                var il = parseFloat(tabinfo.data[x].index_length);
                var di = dl + il;
                var table_type = "&#128202;";
                if (tabinfo.data[x].table_type == "VIEW") {
                    table_type = "&#128065;&#65039;";
                }
                txt += "<tr><td data-sort=\"" + tabinfo.data[x].table_name + "\"><a href=\"#\" onclick=\"document.getElementById('sql').value='SELECT *\\r\\nFROM " + schemaData[sn].schema + "." + tabinfo.data[x].table_name + "\\r\\nLIMIT 1000';decorator.update();return false;\" style=\"text-decoration: none;\">" + table_type + "&nbsp;" + tabinfo.data[x].table_name + "</a></td><td style=\"text-align:right\" data-sort=\"" + tr + "\">" + tr.toLocaleString() + "</td><td style=\"text-align:right\" data-sort=\"" + di + "\">" + dl.toLocaleString() + " / " + il.toLocaleString() + "</td><td style=\"text-align:right\" data-sort=\"" + di + "\">" + di.toLocaleString() + "</td><td>" + tabinfo.data[x].table_comment + "</td></tr>";
            }
            txt += "</tbody></table>";
            document.getElementById('tablelist').innerHTML = txt;
        }
    );
}

var sqlresponse = false;
var resultbody = document.createElement("tbody");
function runSQL() {
    document.getElementById('message').innerHTML = "";
    document.getElementById('results').innerHTML = "";
    document.getElementById('runsql').disabled = true;
    setTimeout(function(){
        if(!sqlresponse) {
            document.getElementById('runsql').disabled = false;
            document.getElementById('message').innerHTML = "<span style='color: red;'>Query Timed out</span>";    
        }
        sqlresponse = false;
    }, 50000);
    wvsql(
        document.getElementById('catalog').value,
        document.getElementById('schema').value,
        document.getElementById('sql').value
    ).then(
        function(res) {
            sqlresponse = true;
            document.getElementById('runsql').disabled = false;
            document.getElementById('message').innerHTML = res.message;
            if (res.result) {
                var fragment = document.createDocumentFragment();
                var table = document.createElement("table");
                resultbody = document.createElement("tbody");
                table.id = "resultstable";
                table.className = "sortable";
                table.style = "font-size:75%;display: none;";
                var thead = document.createElement("thead");

                var tr = document.createElement("tr");
                for (var columnname in res.columns.columnname) {
                    var th = document.createElement("th");
                    th.appendChild(document.createTextNode(columnname));
                    tr.appendChild(th);
                }
                thead.appendChild(tr);
                table.appendChild(thead);
                table.appendChild(resultbody);
                fragment.appendChild(table);
                document.getElementById('results').appendChild(fragment);
                for (var columns in res.data) {
                    var tr = document.createElement("tr");
                    for (var column in res.data[columns]) {
                        var val = res.data[columns][column];
                        if (typeof val == 'number') {
                            var td = document.createElement("td");
                            td.style = "text-align: right;";
                            td.appendChild(document.createTextNode(val));
                            tr.appendChild(td);
                        } else {
                            var td = document.createElement("td");
                            td.appendChild(document.createTextNode(atob(val)));
                            tr.appendChild(td);
                        }
                    }
                    resultbody.appendChild(tr);
                }
                document.getElementById('resultstable').style = "font-size:75%;display: table;";
            }
        }
    );
}