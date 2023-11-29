#include <bits/stdc++.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctime>

using namespace std;

#define buf_size 10240

struct userinformation{
  string userid;
  string password;
  string userip;
  long long int portno=0;
  bool login_status=false;
};

struct groupinformation{

   string groupid;
   string ownerid;
};


struct fileinformation{
  string filename; //assuming file name will be unique
  long long int  filesize;
  string fullFileSha;
  vector<string> chunkWiseSha;
  long long int no_of_file_chunks;
};

struct seederinformation{
  string seedername;
  string filename;
  string filepath;
};



unordered_map<string,struct userinformation> user_user_information;
unordered_map<string,struct fileinformation> file_file_information;
unordered_map<string,struct groupinformation> group_group_information;

unordered_map<string,set<string>> remaining_invites;
unordered_map<string,set<string>> users_in_group;
unordered_map<string,set<string>> files_in_group;
unordered_map< string,set<string>> list_of_seeders;//file name to users
// unordered_map< string,vector<seederInfo  > > list_of_seedersInfo;//file name to seeder info

map<pair<string,string>,string> seeder_file_to_path;
vector<string> show_all_downloads;

string tracker_information_path;
string log_file="./mytrackerLog1";
bool starting_log_file=true;
long long int tracker_number;
vector<string> ip_port_of_trackers;
string tracker_1_ip;
string tracker_2_ip;
string tracker_1_port;
string tracker_2_port;
mutex seedfile_mutex, logfile_mutex;
string seperator = "|*|";
vector<thread> vector_of_threads;
long long int threads_count;
int one=1,zero=0;
long long int backlog=2000;
long long int BUFFER_SIZE=10240;


vector<string> stringparse(string s,char del);
int process_arguments(char *argv[]);
// fstream getLogFile();
// void writeLog(string message);
int peerservice(long long int clientSocketDes,string ip,long long int port);
int createnewuser(string userId,string passwordword,long long int clientSocketDes);
int login(string userId,string passwor,string userip,long long int userport,long long int clientSocketDes);
int creategroup(string userId,string groupId,long long int clientSocketDes);
int joingroup(string groupId,string userId,long long int clientSocketDes);
int  leavegroup(string groupId,string userId,long long int clientSocketDes);
int  listgroups(long long int clientSocketDes);
int  acceptrequests(long long int clientSocketDes,string groupId,string userId,string currentUser);
int  listrequests(long long int clientSocketDes,string groupId,string currentUser);
int listfiles(long long int clientSocketDes,string group_id);
void getseeders(long long int clientSocketDes,string file_id,string group_id,string curr);


int main(int argc,char *argv[]){
  if(argc!=(3*one+zero)){
        cout << "Number of arguments not correct" << endl;
        // cout<<" please run the tracker file in this format :: ./tracker​ tracker_info.txt ​ tracker_number tracker_info.txt "<<endl;     
        exit(one);
    }
    else{
      // cout<<" came here1 "<<endl;
      process_arguments(argv);
      //writeLog("Processed the agruments.");


      struct sockaddr_in tracker_1_address;
      struct sockaddr_in other_address;
        int socket_descriptor = zero;//socket descriptor
        int opt=one;
        socklen_t size;
        if((socket_descriptor=socket(AF_INET,SOCK_STREAM,zero))==zero){
            // writeLog("Socket creation error");
            perror("Unable to connect to socket");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        {
            // writeLog("setsockopt Error");
            exit(EXIT_FAILURE);
        }
        tracker_1_address.sin_family = AF_INET;
        tracker_1_address.sin_port = htons(stoi(tracker_1_port));
        tracker_1_address.sin_addr.s_addr = inet_addr(tracker_1_ip.c_str());

        if (bind(socket_descriptor, (struct sockaddr *)&tracker_1_address, sizeof(tracker_1_address)) < zero){
           perror("failed to bind");
           exit(EXIT_FAILURE);
        }
        // writeLog("Bind Successful.");
        if (listen(socket_descriptor, backlog) < 0){
            perror("error backlog overflow");
            exit(EXIT_FAILURE);
        }
        // writeLog("Socket Created with socket Descriptor = " + to_string(socket_descriptor) + ".");
        // writeLog("Listening for clients.");
        long long int client_socket;
        long long int addrlen = sizeof(tracker_1_address);

        while((client_socket = accept(socket_descriptor, (sockaddr *)&tracker_1_address, (socklen_t *)&addrlen))!=-(one*zero+one)){
        string ip=inet_ntoa(other_address.sin_addr);
        long long int port=(ntohs(other_address.sin_port));
        cout<<" Serving request which came from client"<<endl;
        // cout<<"Request came from client ip "<<other_address.sin_addr<<" port "<<other_address.sin_port<<endl;
        // writeLog("Connection came from client ip "+ip +" port "+ to_string(port)+ ".");
        // try{
             // writeLog("Thread Created for new client."); 
        vector_of_threads.push_back(thread(peerservice,client_socket,ip,port));
        size=sizeof(struct sockaddr);
      //  }
        // catch (const exception &ex){
        //         // writeLog("Thread exited with some exception. :(");
        //     }
      }
      vector<thread>:: iterator it;
      for(it=vector_of_threads.begin();it!=vector_of_threads.end();it++)

      {
        if(it->joinable()) 
         it->join();
        }
 

      // {
      //   if(it->joinable()) 
      //     it->

    cout<<"returning form Tracker main"<<endl;
       
    }
}

vector<string> stringparse(string s,char colon)
{
  //cout<<"my string parser"<<endl;
  stringstream ss(s);
  vector<string> a;
  string temp;
  getline(ss,temp,colon);
  do
  {
    //cout<<"parsing string"<<endl;
    a.push_back(temp);
  }while(getline(ss,temp,colon));
  return a;
}

int process_arguments(char *argv[]){
      string tracker_information_path=argv[one*one+zero];
      tracker_number=stoi(argv[one+one+zero]);
      cout<<"Tracker_no:"<<tracker_number<<endl;     
      fstream serverfilestream(tracker_information_path,ios::in);
       
      string temp;
      getline(serverfilestream,temp,'\n');
      do{
        ip_port_of_trackers.push_back(temp);      
      }while(getline(serverfilestream,temp,'\n'));
      vector<string> iport;
      iport=stringparse(ip_port_of_trackers[zero*one],':');
      tracker_1_ip=iport[zero+zero];
      tracker_1_port=iport[one*zero+one];
      iport=stringparse(ip_port_of_trackers[one*one],':');
      tracker_2_ip=iport[zero*zero*one];
      tracker_2_port=iport[one+one-one];
      return 0;
}

// fstream getLogFile()
// {
//     logfile_mutex.lock();
//     fstream my_file;
//     if(starting_log_file == false){
//        my_file.open(log_file, ios::app);
//     }else {
//       my_file.open(log_file, ios::out);
//       starting_log_file = false;
//     }
   
//     return my_file;
// }

// void writeLog(string message)
// {
//     fstream logfile_fd;
//     logfile_fd = getLogFile();
//     time_t cur = time(NULL);
//     string t = ctime(&cur);
//     t = t.substr(one*4,one*4*4);
//     logfile_fd << t << ": " << message << endl;
//     logfile_mutex.unlock();
//     return;
// }

int createnewuser(string user_id,string passwor,long long int clientSocketDes){
  if(user_user_information.find(user_id)==user_user_information.end())
  {
    char status[]="1";
    userinformation user;
    user.userid=user_id;
    user.password=passwor;
    user_user_information[user_id]=user;
    // cout<<user_user_information[userId].userId<<endl;
    // cout<<user_user_information[userId].passwordword<<endl;
    long long int s=sizeof(status);
    send(clientSocketDes,status,s,zero*one);
  }
  else
  {
    char status[]="0";
    long long int s=sizeof(status);
    send(clientSocketDes,status,s,zero*zero);

  }
  close(clientSocketDes);
  return 0;
}

int login(string userId,string passwor,string user_ip,long long int userport,long long int clientSocketDes){
  if((user_user_information.find(userId)!=user_user_information.end())&&(user_user_information[userId].password==passwor)){
    cout<<" loging in "<<userId <<endl;
    user_user_information[userId].userip=user_ip;
    user_user_information[userId].portno=userport;
    user_user_information[userId].login_status=true;
    char status[]="1";    
    send(clientSocketDes,status,sizeof(status),zero*one);
  }
  else{
    char  status[]="0";
    send(clientSocketDes,status,sizeof(status),zero*zero);
  }
  close(clientSocketDes);
  return 0;
}

int peerservice(long long int clientSocketDes,string ip,long long int port){
  // cout<<" Okay serve request is working "<<endl;
  
        char buffer[BUFFER_SIZE] = {0}; 
        long long int valread = read( clientSocketDes , buffer, BUFFER_SIZE); 
        string request=buffer;
        vector<string> request_to_serve=stringparse(request,';');
        string command_given= request_to_serve[zero*one+zero];
  long long int choice=-1;
  if(command_given=="create_user"){
    choice=one*zero+one;
  } 

  else if(command_given=="login"){
     choice=one*one+one;
  } 

  else if(command_given=="create_group"){
     choice=one+one*one+one;
  }
  else if(command_given=="join_group"){
      choice=one*3+one;
  }
  else if(command_given=="leave_group"){
      choice=one+one*4;
  }
  else if(command_given=="list_requests"){
     choice=one*4+one+one;
  }
  else if(command_given=="accept_request"){
      choice=one*3+one*4;
  }
  else if(command_given=="list_groups"){
   choice=one*3+one*5;                              //8
  }
  else if(command_given=="list_files"){
    choice=one*9;                                   //9
  }
  else if(command_given=="upload_file"){
    choice=one*8+one*2;                              //10
  }
  else if(command_given=="logout"){
     choice=one*10+zero+one;                         //11
  }
  else if(command_given=="show_downloads")           //12
  {
    choice=one*9+one+one+one;
  }
  else if(command_given=="add_to_seeder_list"){       //13
    choice=one*13;
  }

  else if(command_given=="get_active_seeders")        //14
  {
    choice=one*13+one;   
  }

  string user_id;
  string group_id;
  string user_ip;
  string passwor;
  string curr;
  string info;
  string FileId;
  string FilePath;


  switch (choice) {
  case 1:
  {
      user_id=request_to_serve[zero+one];
      passwor=request_to_serve[zero+one+one*one];
      createnewuser(user_id,passwor,clientSocketDes);
    break;
  }
  case 2:
  {
        user_id=request_to_serve[one*one-zero];
        passwor=request_to_serve[one*one+one];
        user_ip=request_to_serve[one+one+one];
       long long int userport=stoi(request_to_serve[4]);
       login(user_id,passwor,user_ip,userport,clientSocketDes);
    break;
  }
  case 3:
  {
        group_id=request_to_serve[one*one];
        user_id=request_to_serve[one+one];
       creategroup(user_id,group_id,clientSocketDes);
    break;
  }
  case 4:
  {
        group_id=request_to_serve[one*one];
        user_id=request_to_serve[one*one+one];
       joingroup(group_id,user_id,clientSocketDes);
    break;
  }
  case 5:
  {
        group_id=request_to_serve[one];
        user_id=request_to_serve[one+one];
       leavegroup(group_id,user_id,clientSocketDes);
    break;
  }
  case 6:
  {
        group_id=request_to_serve[1];
        curr=request_to_serve[2];
       listrequests(clientSocketDes,group_id,curr);
    break;
  }
  case 7:
  {
       group_id=request_to_serve[one];
       user_id=request_to_serve[one+one];
       curr=request_to_serve[one+one+one];
      int j=one+one+one;
      acceptrequests(clientSocketDes,group_id,user_id,curr);
    break;
  }
  case 8:
  {
        listgroups(clientSocketDes);
    break;
  }
  case 9:
  {
         group_id=request_to_serve[one];
        listfiles(clientSocketDes,group_id);
    break;
  }
  case 10:
  {
     info="";
     group_id=request_to_serve[one+one+one];
     user_id=request_to_serve[one+one*one+one+one]; 
     FileId=request_to_serve[one*one+one];


      if(group_group_information.find(group_id)!=group_group_information.end()){
        if(users_in_group[group_id].find(user_id)!= users_in_group[group_id].end()){
          if(files_in_group[group_id].find(FileId)==files_in_group[group_id].end()){
            files_in_group[group_id].insert(FileId);
          }
          list_of_seeders[FileId].insert(user_id);
         
         fileinformation f_info;
         f_info.filename=FileId;
         f_info.filesize=stoull(request_to_serve[one+one+one+one+one]);
         f_info.fullFileSha=request_to_serve[7];
         f_info.no_of_file_chunks=stoi(request_to_serve[one+one+one+one+one+one]);
         for(long long int i=one*8;i<request_to_serve.size();){
            f_info.chunkWiseSha.push_back(request_to_serve[i]);
            i++;
          }
          
          file_file_information[request_to_serve[one+one]]=f_info;

        seeder_file_to_path[make_pair(user_id,FileId)]=request_to_serve[zero+one];
          // cout<<" file seeder to file path "<<endl;
          cout<<seeder_file_to_path[make_pair(user_id,FileId)]<<endl;
         
           cout<<"Uploading "<<group_id<<" "<<FileId<<" "<<user_user_information[user_id].portno<<endl;

        }
        else{
          info=info+"User does not belong to this group";
          send(clientSocketDes,info.c_str(),strlen(info.c_str()),zero+zero);
        }
      }
      else{
        info=info+"the group does not exist";
        send(clientSocketDes,info.c_str(),strlen(info.c_str()),one-one);
      }
    break;
    }
    case 11:
    {
       string userId=request_to_serve[1];
       user_user_information[userId].login_status=false;
    break;
    }
    case 12:
    {
      info="";
      if(show_all_downloads.size()>(zero*zero)){
         for(int i=0;i<show_all_downloads.size()-1;i++){
          info+=show_all_downloads[i];
          info+=";";
      }
      info+=show_all_downloads[show_all_downloads.size()-one];
      send(clientSocketDes,info.c_str(),info.size(),one-one);
      }
    break;
    }
    case 13:
    {
       group_id=request_to_serve[1];
        FileId=request_to_serve[2];
       FilePath=request_to_serve[3];
        user_id=request_to_serve[4];
       list_of_seeders[FileId].insert(user_id);
        cout<<"adding user to seeder list "<<user_id<<endl;
         info="C "+group_id+" "+FileId;
        // cout<<" info "<<info<<endl;
        show_all_downloads.push_back(info);


       seeder_file_to_path[make_pair(user_id,FileId)]=FilePath;
    break;
    }
    case 14:
    {
           string currentUser=request_to_serve[one+one+one];
    
      group_id=request_to_serve[one+one];
    
      string file_id=request_to_serve[one];
     getseeders(clientSocketDes,file_id,group_id,currentUser);

    break;
    }
}  
 
  return 0;
}

int creategroup(string userId,string groupId,long long int clientSocketDes){
  if(group_group_information.find(groupId)==group_group_information.end()){
    char status[]="1";
    groupinformation grp;
    grp.groupid=groupId;
    grp.ownerid=userId;
    group_group_information[groupId]=grp;
    // cout<<group_group_information[groupId].group_id<<"  "<<group_group_information[groupId].ownwerId<<endl;
    users_in_group[groupId].insert(userId);
    send(clientSocketDes,status,sizeof(status),zero*zero);
  }
  else{
    char status[]="0";
    send(clientSocketDes,status,sizeof(status),zero);
  }
  close(clientSocketDes);
  return 0;
}

int joingroup(string groupId,string userId,long long int clientSocketDes){

    if(users_in_group.find(groupId)!=users_in_group.end()){
     if(users_in_group[groupId].find(userId)==users_in_group[groupId].end()){
      remaining_invites[groupId].insert(userId);
      char status[]="1";
      send(clientSocketDes,status,sizeof(status),one-one);
     }
     else{
          char status[]="2";
          send(clientSocketDes,status,sizeof(status),zero+zero);
     }      
  }
  else{
    char status[]="0";
    send(clientSocketDes,status,sizeof(status),zero*zero);
  }
  close(clientSocketDes);
  return 0;
}

int leavegroup(string groupId,string userId,long long int clientSocketDes){
    if(group_group_information.find(groupId)==group_group_information.end()){   
          char status[]="0";
          send(clientSocketDes,status,sizeof(status),zero*zero);
  }
  else{
    if(group_group_information[groupId].ownerid == userId){
             group_group_information.erase(groupId);
             remaining_invites.erase(groupId);
             users_in_group.erase(groupId);
             files_in_group.erase(groupId);
             char status[]="1";
             send(clientSocketDes,status,sizeof(status),0);

    }
    else{
       if(users_in_group[groupId].find(userId)!=users_in_group[groupId].end()){
      char status[]="1";
      users_in_group[groupId].erase(userId);
      send(clientSocketDes,status,sizeof(status),0);
     } 
    }
  }
  close(clientSocketDes);
  return 0;
}

int listgroups(long long int clientSocketDes){
  string info="";
  for(auto &i:group_group_information)
  {
      info+=i.first;
      info+=";";
  }
  send(clientSocketDes,info.c_str(),strlen(info.c_str()),zero*zero);
  close(clientSocketDes);
  return 0;
}

int acceptrequests(long long int clientSocketDes,string groupId,string userId,string currentUser){

  if(currentUser == group_group_information[groupId].ownerid){
    remaining_invites[groupId].erase(userId);
    users_in_group[groupId].insert(userId);
    string info=userId+" is now a part of group "+groupId;
    send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);   
  }
  else{
    string info="You are not the owner of this group ";
    send(clientSocketDes,info.c_str(),strlen(info.c_str()),zero*zero);
  }
  
  close(clientSocketDes);
  return 0;
}


int  listrequests(long long int clientSocketDes,string groupId,string currentUser){
  if(currentUser == group_group_information[groupId].ownerid){
    string info="";
    if(remaining_invites.find(groupId) == remaining_invites.end()){
      send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);
    }
    else{
      string b=":";
      for(auto i:remaining_invites[groupId]){
          info+= i;
          info+=b;
      }
      send(clientSocketDes,info.c_str(),strlen(info.c_str()),zero*zero);
    }
  }
  else{
   string info="You are not the owner of this group ";
    send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);

  }
  close(clientSocketDes);
  return 0;
}

int listfiles(long long int clientSocketDes,string group_id){
  string info="";
  for(auto i:files_in_group[group_id]) {
          info+= i;
          info+=";";
    }
  send(clientSocketDes,info.c_str(),strlen(info.c_str()),one*zero);
  return 0;
}


void getseeders(long long int clientSocketDes,string file_id,string group_id,string curr){
 // cout<<" getting seeders "<<endl;
  string info="";
  char status[]={0};

  //valid group
  //check user in group
  //check file in group

  if(group_group_information.find(group_id) == group_group_information.end()){
    cout<<"Invalid group"<<endl;
    send(clientSocketDes,status,1,0);
  }else{
    if(users_in_group[group_id].find(curr) == users_in_group[group_id].end()){
      cout<<"Invalid user "<<endl;
      send(clientSocketDes,status,1,0);
    }else {
      if(files_in_group[group_id].find(file_id) == files_in_group[group_id].end()){
        cout<<"No such fiile "<<endl;
        send(clientSocketDes,status,1,0);
      }else{

        status[0]='1';
        // cout<<" sending ports of active seeders "<<endl;
        send(clientSocketDes,status,sizeof(status),0);
        // cout<<"size "<<list_of_seeders[file_id].size()<<endl;

        vector<string> seeders_name;
        for(auto i:list_of_seeders[file_id]){
          seeders_name.push_back(i);
          // cout<<i<<endl;
        }
        vector <string> ip_port_active;
        info+=to_string(seeders_name.size());
        info+=";";
        for(int i=0;i<seeders_name.size();i++){
          if(user_user_information[seeders_name[i]].login_status == true){
            string ip=user_user_information[seeders_name[i]].userip;
          int port=user_user_information[seeders_name[i]].portno;
          
          string seedpath=seeder_file_to_path[make_pair(seeders_name[i],file_id)];
          string ip_port=ip+":"+to_string(port)+":"+seeders_name[i]+":"+seedpath;
          ip_port_active.push_back(ip_port);
          }
          
          
        }
        for(int i=0;i<ip_port_active.size()-1;i++){
          
          info+=ip_port_active[i];
          info+=";";
          
          
        }
        info+=ip_port_active[ip_port_active.size()-1];
          
      send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);

      info="";
      fileinformation file=file_file_information[file_id];
      info+=to_string(file.filesize);
      info+=";";
      info+=file.fullFileSha;
      info+=";";
      info+=to_string(file.no_of_file_chunks);
      info+=";";

      for(int i=0;i<file.no_of_file_chunks-1;i++){
        info+=file.chunkWiseSha[i];
        info+=";";
      }
      info+=file.chunkWiseSha[file.no_of_file_chunks-1];
      // cout<<info<<endl;
      send(clientSocketDes,info.c_str(),strlen(info.c_str()),0);

      }
    }
  }
  
}
