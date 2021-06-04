/* WARNING: Do NOT forget to terminate your lines with ; otherwise it breaks in strange ways */
var schemaData;

function doLogin() {
    document.getElementById('loginButton').disabled = true;
    var db = {
        server:   document.getElementById('server').value,
        login:    document.getElementById('login').value,
        password: document.getElementById('password').value
    };
    server = document.getElementById('server').value;
    login = document.getElementById('login').value;
    password = document.getElementById('password').value;
    wvlogin(
        db
    ).then(
        function(res) {
            document.getElementById('loginButton').disabled = false;
            document.getElementById('message').innerHTML = res.message;
            if (res.result) {
                document.getElementById('loginview').style.visibility = "hidden";
                document.getElementById('dbview').style.visibility = "visible";
                document.getElementById('header').innerHTML = "Connected to mysql://" + login + "@" + server + "/ Version: " + res.version + " Schemas: " + res.data.length + ", Tables: " + res.tables_tot + ", MB: " + res.mb_tot + " <button onclick=\"doLogout()\">Logout</button>";
                document.getElementById('footer').innerHTML = res.message;
                schemaData = res.data;
                var schemas = document.getElementById('schemas');
                document.getElementById('schemas').length = 0;
                for (var i = 0; i < res.data.length; i = i + 1) {
                    var schema = document.createElement("li");
                    var a = document.createElement('a');
                    var progress = document.createElement('progress');
                    progress.max = parseInt(res.mb_max);
                    progress.value = parseInt(res.data[i].mb);
                    var linkText = document.createTextNode(res.data[i].schema);
                    a.appendChild(linkText);
                    a.appendChild(progress);
                    a.href = "javascript:void(0)";
                    a.sn = i;
                    a.onclick = function () {
                        document.getElementById('content').innerHTML = JSON.stringify(schemaData[this.sn], null, 4);
                        wvtables(
                            schemaData[this.sn].catalog,
                            schemaData[this.sn].schema
                        ).then(
                            function(tabinfo) {
                                var txt = "<table class='sortable'><thead><th>Table Name</th><th>Type</th><th>Engine</th><th>Rows</th><th>Data Length</th><th>Index Length</th><th>Total Length</th><th>Comment</th></thead></tr><tbody>";
                                for (var x in tabinfo.data) {
                                    txt += "<tr><th>" + tabinfo.data[x].table_name + "</th><td>" + tabinfo.data[x].table_type + "</td><td>" + tabinfo.data[x].engine + "</td><td>" + tabinfo.data[x].table_rows + "</td><td>" + tabinfo.data[x].data_length + "</td><td>" + tabinfo.data[x].index_length + "</td><td>" + (parseFloat(tabinfo.data[x].data_length) + parseFloat(tabinfo.data[x].index_length)) + "</td><td>" + tabinfo.data[x].table_comment + "</td></tr>";
                                }
                                txt += "<tbody></table>";
                                document.getElementById('content').innerHTML += ",\n" + txt;
                            }
                        );
                        return false;
                    };
                    schema.appendChild(a);
                    schemas.appendChild(schema);
                }
            }
        }
    );
}

function doLogout() {
    document.getElementById('dbview').style.visibility = "hidden";
    document.getElementById('loginview').style.visibility = "visible";
    document.getElementById('message').innerHTML = "";
    wvlogout().then(
        function(res) {
            document.getElementById('message').innerHTML = res.message;
        }
    );
}
