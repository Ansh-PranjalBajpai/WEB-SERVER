#include "client_handler.h"
using namespace std;

vector<string> split(string &request, string delim)
{
    stringstream ss (request);
    vector<string> comps;
    string temp;
    while(getline(ss, temp, delim[0]))
    {
      comps.push_back(temp);
      ss.ignore(delim.size()-1);
    }
    return comps;
}

bool case_insen_starts_with(string &a,string b)
{
    if (b.size()>a.size())
         return false;
    int n = b.size();
    for (int i=0;i<n;i++)
    {
        if (b[i]!=a[i]) return false;
    }
    return true;
}

string gzip_compress(const string &data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        throw runtime_error("deflateInit2 failed while compressing.");
    }

    zs.next_in = (Bytef *)data.data();
    zs.avail_in = data.size();

    int ret;
    char outbuffer[32768];
    string outstring;

    do {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        throw runtime_error("Exception during zlib compression: (" + to_string(ret) + ") " + zs.msg);
    }

    return outstring;
}

void handleClient(int client, string directory)
{
  char buffer[4096];
  int bytes_received = recv(client, buffer, sizeof(buffer) - 1, 0);
  buffer[bytes_received] = '\0'; 
  string request(buffer);
  vector<string> comps = split(request, "\r\n");
  vector<string> comps2 = split(comps[0], " ");
  string method = comps2[0];
  string endpoint = comps2[1];
  string response="";
  if (endpoint=="/")
      response = "HTTP/1.1 200 OK\r\n\r\n";
  else if (endpoint.starts_with("/echo"))
  {
    string encoding = "";
    for (auto &elem: comps)
    {
        if (case_insen_starts_with(elem, "Accept-Encoding"))
        {
            cout<<"hi\n";
            vector<string> temp = split(elem, " ");
            for (auto &encod: temp)
            {
                if (case_insen_starts_with(encod,"gzip")) encoding = "gzip";
            }
            break;
        }
    }
    if (encoding!="gzip")
    {
        string text= endpoint.substr(6);
        response ="HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(text.size()) + "\r\n\r\n" + text;
    }
    else
    {
        cout<<encoding<<"\n";
        string text= endpoint.substr(6);
        text = gzip_compress(text);
        response ="HTTP/1.1 200 OK\r\nContent-Encoding: "+ encoding +"\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(text.size()) + "\r\n\r\n" + text;
    }      
  }
  else if (endpoint.starts_with("/user"))
  {
    string text;
      for (auto &elem:comps)
      {
        if (elem.starts_with("User"))
        {
          vector<string> temp = split(elem, " ");
          text = temp[1];
          break;
        }
      }
      response ="HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(text.size()) + "\r\n\r\n" + text;
  }
  else if (endpoint.starts_with("/files"))
  {
    string filepath = directory+endpoint.substr(6);
    if (method=="GET")
    {
        ifstream file(filepath);
        if (file)
        {
            stringstream content;
            content << file.rdbuf();
            file.close();
            string text = content.str();
            response = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: " + to_string(text.size()) + "\r\n\r\n"+ text;
        }
        else
        {
            response = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        }
    }
    else if (method=="POST")
    {
        string text = comps[comps.size()-1];
        ofstream outfile(filepath);

        if (outfile.is_open()) 
        {
            outfile << text;
            outfile.close();
            response = "HTTP/1.1 201 Created\r\n\r\n";
            cout << "Content saved to 'output.txt' successfully.\n";
        } 
        else 
        {
            cerr << "Error opening the output file.\n";
            response = "HTTP/1.1 404 Not Found\r\n\r\n";
        }
    }
    
  }
  else
  {
      response = "HTTP/1.1 404 Not Found\r\n\r\n";
  }

  send(client, response.c_str(), response.length(), 0);
  cout << "Client connected\n";
  close(client);
}
