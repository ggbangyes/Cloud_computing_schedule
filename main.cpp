#include <iostream>
#include <algorithm>
#include <stack>
#include <vector>

#define n_core 3
#define n_task 10
using namespace std;

void Primary_assignment(int t_l_k[][n_core], int n, int k, int t_re, int C[]){
    int t_l_min[n];
    int i;
    for(i=0;i<n;i++){
        t_l_min[i] = t_l_k[i][k-1];
    }
    //cout<<"where to execute task i ?   0 in local, and 1 in cloud:\n";
    for(i=0; i<n; i++){
        if(t_re < t_l_min[i])
            C[i]= 1;        //cloud
        else
            C[i]= 0;       //local
        //cout<<C[i] <<" ";
    }

}

void Task_prioritizing(int t_l_k[][n_core],int taskGraph[][n_task], float Priority[], int PriOrder[], float W[], int C[], int k, int n, int t_re){
    int i,j;
    float max_pri;
    for(i=0; i<n; i++){
        if(C[i]==1){  //if in cloud
            W[i] = float(t_re);
        }
        else{    //if in local
            W[i] = float(t_l_k[i][0]+ t_l_k[i][1]+ t_l_k[i][2])/k;
        }
        //cout<<W[i] <<" ";
    }
    cout<<"\ntask priority, from task10 to task1:\n";
    //calculate priority of each task
    Priority[n-1] = W[n-1];
    for(i=n-1; i>=0; i--){
        max_pri=0;
        for(j=0; j<n; j++){
            if(taskGraph[i][j]==1 && Priority[j] >max_pri){
                max_pri = Priority[j];
            }
        }
        Priority[i] = W[i]+max_pri;

        cout<<Priority[i]<<" ";
    }

    //pair: first element is priority, second element is index of task
    vector< pair<float, int> > vect;
    for(i=0; i<n; i++){
        vect.push_back(make_pair(Priority[i], i));
    }
    sort(vect.begin(), vect.end());  //priority from low to high
    cout<<"\n"<<"task index, priority form low to high:\n";

//    for(i=0; i<n; i++){
//        cout<<vect[i].first<<" "<<vect[i].second+1;
//        cout<<"\n";
//    }
    for(i=0; i<n; i++){
        PriOrder[i] = vect[i].second;
        cout<<"task"<<PriOrder[i]+1<<" ";
    }
    cout<<"\n";

}

void Unit_selection(int n, int k, int PriOrder[], int t_l_k[][n_core], int taskGraph[][n_task], int core_time[],
                    int rt_l[], int ft_l[], int ft_wr[], int ft_ws[], int rt_ws[], int rt_c[], int ft_c[], int ft[],
                    int t_s, int t_c, int t_r, int task_loc[]){
    int i, j, task, max_rt_l, ft_min, max_rt_ws, max_rt_c, max_ft_c;
    int core_index;
    for(i=n-1; i>=0; i--){
        task=PriOrder[i];
        max_rt_l = 0;
        //##################### 计算local的最短ft #####################

        //******* ready time in local *********
        for(j=0; j<n; j++){
            if(taskGraph[j][task]==1 && max(ft_l[j], ft_wr[j]) > max_rt_l){
                max_rt_l = max(ft_l[j], ft_wr[j]);
            }
        }
        rt_l[task]= max_rt_l;

        //******* finish time in local *********
        ft_min=INT_MAX;
        core_index = -1;
        //core上的最短ft: ready time 与 core time 选出大的那个，加上core上的运行时间,
        for(j=0; j<k; j++){
            if(max(core_time[j], rt_l[task]) + t_l_k[task][j] < ft_min) {
                ft_min = max(core_time[j], rt_l[task]) + t_l_k[task][j];
                core_index = j;
                //cout<<"  "<<t_l_k[task][j] <<" "<<ft_min;
            }
        }
        ft_l[task] = ft_min;


        //##################### 计算cloud的最短ft #####################

        //******* ws ready time and finish time  *********
        max_rt_ws=0;
        //所有predecessor中，max（local的ft， 和无线传输的ft）。
        for(j=0; j<n; j++){
            if(taskGraph[j][task]==1 && max(ft_l[j], ft_ws[j]) > max_rt_ws){
                max_rt_ws = max(ft_l[j], ft_ws[j]);
            }
        }
        rt_ws[task] = max_rt_ws;
        ft_ws[task] = max(rt_ws[task], core_time[3]) + t_s;

        //******* cloud computing ready time and finish time *********
        max_rt_c=0;
        max_ft_c=0;
        // max(该任务的无线传输ft， 和max（所有predecessor中 cloud的ft））。
        for(j=0; j<n; j++){
            if(taskGraph[j][task]==1 &&  ft_c[j]> max_ft_c){
                max_ft_c =  ft_c[j];
            }
        }
        rt_c[task] = max(ft_ws[task] ,max_rt_c);

        //******* finish time cloud and finish time wr *********
        ft_c[task] = rt_c[task] + t_c;
        ft_wr[task] = ft_c[task] + t_r;

        //############ 判断shcedule到local 还是cloud ##############
        if(ft_l[task] <= ft_wr[task]){ //local
            ft[task] = ft_l[task];
            core_time[core_index] = ft[task];
            ft_ws[task] = 0;
            ft_c[task] = 0;
            ft_wr[task] = 0;
            //core_list[core_index].push_back(task);
            task_loc[task] = core_index;
            //cout<<"task "<<task+1 <<" in core: " <<core_index+1<<" finish time: "<<ft[task]<<"\n";
        }
        else{ //cloud
            ft[task] = ft_wr[task];
            ft_l[task] = 0;
            //数组的最为一个用于记录在cloud的信息
            core_time[n_core] = ft[task]-t_r-t_c;  //cloud上的 ws 完成时间
            //core_list[n_core].push_back(task);
            task_loc[task] = n_core;
            //cout<<"task "<<task+1 <<" in cloud"<<" finish time: "<<ft[task]<<"\n";
        }
    }



}

void Kernel_algorithm(vector< vector<int> > core_list, int t_l_k[][n_core], int taskGraph[][n_task], int task_loc[], int init_ft[], int init_st[]){

    int con1, con2, i, j;
    int ft[n_task],st[n_task], rt[n_task]={0};
    int  task_loc_now[n_task];
    //step 1 中初始调度的信息
    for(i=0; i<n_task; i++){
        ft[i] = init_ft[i];
        st[i] = init_st[i];
        task_loc_now[i] = task_loc[i];
    }
    vector<vector<int>> clist(n_core+1);
    for(i=0; i<core_list.size(); i++){
        for(j=0; j<core_list[i].size(); j++){
            clist[i].push_back(core_list[i][j]);
        }
    }

    //############### 考虑每个task的调度 ################
    for(con1=0; con1<n_task; con1++){
        if(task_loc[con1]==n_core)//if task already schedule in cloud dont consider it again
            break;


        //################# 考虑调度到每个core ###############
        for(con2=0; con2<n_core+1; con2++){
            if(task_loc_now[con1]==con2)//if task already schedule in this core
                break;

            int ft_now[n_task], rt_target[n_task], rt_now[n_task];

            //调度一个任务的初始化
            for(i=0; i<n_task; i++){
                ft_now[i] = ft[i];

            }
            vector<vector<int>> tlist(n_core+1); //储存目前的调度队列
            for(i=0; i<clist.size(); i++){
                for(j=0; j<clist[i].size(); j++){
                    tlist[i].push_back(clist[i][j]);
                }
            }

            //删除target task在原序列中的位置
            for(i=0; i<tlist.size();i++){
                for(j=0; j<tlist[i].size(); j++){
                    if(tlist[i][j] == con1)
                        tlist[i].erase(tlist[i].begin()+j);
                }
            }
            //calculate the rt of target task
            int max_rt =0;
            for(i=0; i<n_task; i++){
                if(taskGraph[i][con1]==1 && max_rt <ft_now[i] )
                    max_rt = ft_now[i];
            }
            rt_target[con1] = max_rt;

            //目标任务插入到新序列
            task_loc_now[con1] = con2; //记录当前task在哪个core
            //specify the insert location
            int loc1 = 0;
            for(i=0; i<tlist[con2].size(); i++){
                if(st[i] > rt_target[con1]){
                    loc1 = i;
                    break;
                }
            }
            tlist[con2].insert(tlist[con2].begin()+loc1,con1);
            //print current clist
            for(i=0; i<tlist.size(); i++){
                cout<<"core "<<i+1<<": ";
                for(j=0; j<tlist[i].size(); j++){
                    cout<<tlist[i][j]+1<<" ";
                }
                cout<<"\n";
            }

            // ###initialize ready1 and ready2 ########
            int ready1[n_task]={0}, ready2[n_task]={0};
            for(i=0; i<n_task; i++){
                for(j=0; j<n_task; j++){
                    if(taskGraph[j][i]==1)
                        ready1[i]= ready1[i]+1;
                }
            }
            for(i=0; i<clist.size(); i++){
                for(j=0; j<tlist[i].size(); j++){
                    ready2[tlist[i][j]]= j;
                }
            }
            //print ready1, ready2
            cout<<"ready1: ";
            for(i=0; i<n_task; i++){
                cout<<ready1[i]<<" ";
            }
            cout<<"\nready2: ";
            for(i=0; i<n_task; i++){
                cout<<ready2[i]<<" ";
            }

            // ################# stack ########
            stack<int> s;
            int pushed[n_task]={0}; //0表示没入过栈， 1表示入过栈了
            for(i=0; i<n_task; i++){
                if(ready1[i]==0 && ready2[i]==0 && pushed[i]==0){
                    s.push(i);
                    pushed[i]=1;
                }
            }

            while(s.size()!=0){
                int curr_task = s.top();
                s.pop();
                max_rt=0;
                if(task_loc_now[curr_task]==n_core){ //cloud
                    for(i=0; i<n_task; i++){
                        if(taskGraph[i][curr_task]==1 && max_rt <ft_now[i] )
                            max_rt = ft_now[i];
                    }
                    rt_now[curr_task] = max_rt;
                    ft_now[curr_task];

                }


            }

            break;
        }
        break;
    }



}

int kernel( vector<vector<int>> clist, int t_l_k[][n_core], int taskGraph[][n_task],int core1[], int tmax, int t_total, float e_total, int n, int k, int st[], int et[], int E_c, int E_l[][3]){
    int out = 0;
    int count = 0;
    while(out == 0){
        float max_ratio = 0;
        int new_n = 0, new_k = 0, new_index1 = 0, new_index2 = 0, new_t = t_total;
        float new_e = e_total;
        int less_t1 =0, less_t2 = 0;
        int temp_core[n], new_st[n], new_ft[n];;
        for(int i=0; i<n; i++){
            for(int j=0; j<k+1; j++){
                int core2[n], core3[k+1], rt[n], rt1[n], ft[n], ft1[n], pushed[n];
                vector<vector<int>> tlist(k+1);
                int index1, index2 = 0;
                for(int i=0; i<n; i++){
                    rt[i] = 0;
                    ft[i] = 0;
                    core2[i] = core1[i];
                    ft1[i] = et[i];
                    rt1[i] = st[i];
                    pushed[i] = 0;
                }
                for(int a=0; a<clist.size(); a++){
                    //core2[i] = core1[i];
                    core3[a] = 0;
                    for(int b=0; b<clist[a].size(); b++){
                        tlist[a].push_back(clist[a][b]);
                    }
                }
                int current_core = core1[i];

                //################## 选择序列中合适的位置插入 ################
                for(int a=0; a<tlist[current_core].size(); a++){
                    if(tlist[current_core][a] == i){
                        index1 = a;
                    }
                }
                tlist[current_core].erase(tlist[current_core].begin()+index1);
                //caculate the ready time of target task
                if(j == n_core){
                    int max_j_ws = 0;
                    for(int a=0; a<n; a++){
                        if(taskGraph[a][i] == 1 && max_j_ws < ft1[a]){
                            max_j_ws = ft1[a];
                        }
                    }
                    rt[i] = max_j_ws;
                }
                else{
                    int max_j_l = 0;
                    for(int a=0; a<n; a++){
                        if(taskGraph[a][i] == 1 && max_j_l < ft1[a]){
                            max_j_l = ft1[a];
                        }
                    }
                    rt[i] = max_j_l;
                }
                core2[i] = j;
                //cout<<tlist[j].size()<<" "<<count<<endl;
                if(tlist[j].size() == 0){
                    index2 = 0;
                }
                else if(tlist[j].size() == 1){
                    if(rt1[tlist[j][0]] > rt[i]){
                        index2 = 0;
                    }
                    else{
                        index2 = 1;
                    }
                }
                else{
                    if(rt1[tlist[j][0]] > rt[i]){
                        index2 = 0;
                    }
                    else if(rt1[tlist[j][tlist[j].size()-1]] <= rt[i]){
                        index2 = tlist[j].size();
                    }
                    else{
                        for(int b=0; b<tlist[j].size()-1; b++){
                            if(rt[i]>=rt1[tlist[j][b]] && rt[i]<=rt1[tlist[j][b+1]]){
                                index2 = b+1;
                            }
                        }
                    }
                }
                tlist[j].insert(tlist[j].begin()+index2,i);
                //cout<<index2<<" "<<i<<" "<<j<<endl;


                // #################initialize ready1 and ready2 ################
                int ready1[n], ready2[n];
                for(int a=0; a<n; a++){
                    ready1[a] = 0;
                }
                for(int a=0; a<10; a++){
                    for(int b=0; b<10; b++){
                        if(taskGraph[a][b] == 1){
                            ready1[b] += 1;
                        }
                    }
                    ready2[a] = 1;
                }
                //cout<<ready2[0]<<" "<<i<<" "<<j<<endl;
                for(int a=0; a<n_core+1; a++){
                    if(tlist[a].size()>0){
                        ready2[tlist[a][0]] = 0;
                    }
                }


                //#########intialize the stack and implement the first operation#######
                stack<int> s;
                for(int a=0; a<n; a++){
                    if(ready1[a] == 0 && ready2[a] == 0 && pushed[a] == 0){
                        s.push(a);
                        pushed[a] = 1;
                        //cout<<a<<" "<<endl;
                    }
                }
                //cout<<ready2[2]<<" "<<i<<" "<<j<<endl;
                int curr_task1 = s.top();
                s.pop();
                rt[curr_task1] = 0;
                if(core2[curr_task1] == n_core){  //cloud
                    rt[curr_task1] = max(core3[core2[curr_task1]],rt[curr_task1]);
                    ft[curr_task1] = rt[curr_task1] + 5;
                    core3[core2[curr_task1]] = rt[curr_task1] + 3;
                }
                else{  //local
                    rt[curr_task1] = max(core3[core2[curr_task1]],rt[curr_task1]);
                    ft[curr_task1] = rt[curr_task1] + t_l_k[curr_task1][core2[curr_task1]];
                    core3[core2[curr_task1]] = ft[curr_task1];
                }
                //cout<<rt[current1]<<" "<<ft[current1]<<endl;
                //**************** update ready1 and ready2 ******************
                for(int a=0; a<n; a++){
                    if(taskGraph[curr_task1][a] == 1){
                        ready1[a] -= 1;
                    }
                }
                ready2[curr_task1] = 1;
                //cout<<ready1[5]<<" "<<count<<endl;
                if(tlist[core2[curr_task1]].size()>1){
                    for(int a=1; a<tlist[core2[curr_task1]].size(); a++){
                        if(tlist[core2[curr_task1]][a-1] == curr_task1){
                            ready2[tlist[core2[curr_task1]][a]] = 0;
                            //cout<<tlist[core2[current1]][a]<<" "<<i<<" "<<j<<endl;
                        }
                    }
                }
                //cout<<core2[current1]<<" "<<i<<" "<<j<<endl;
                for(int a=0; a<n; a++){
                    if(ready1[a] == 0 && ready2[a] == 0 && pushed[a] == 0){
                        s.push(a);
                        pushed[a] = 1;
                        //cout<<a<<" ";
                    }
                }

                while(s.size() != 0){
                    int curr_task2 = s.top();
                    s.pop();
                    //cout<<current<<" ";
                    //caculate ready time of current task
                    if(core2[curr_task2] == n_core){
                        int max_j_ws1 = 0;
                        for(int a=0; a<n; a++){
                            if(taskGraph[a][curr_task2] == 1 && max_j_ws1 < ft[a]){
                                max_j_ws1 = ft[a];
                            }
                        }
                        rt[curr_task2] = max_j_ws1;
                    }
                    else{
                        int max_j_l1 = 0;
                        for(int a=0; a<n; a++){
                            if(taskGraph[a][curr_task2] == 1 && max_j_l1 < ft[a]){
                                max_j_l1 = ft[a];
                            }
                        }
                        rt[curr_task2] = max_j_l1;
                    }
                    if(core2[curr_task2] == n_core){
                        rt[curr_task2] = max(core3[core2[curr_task2]],rt[curr_task2]);
                        ft[curr_task2] = rt[curr_task2] + 5;
                        core3[core2[curr_task2]] = rt[curr_task2] + 3;
                    }
                    else{
                        rt[curr_task2] = max(core3[core2[curr_task2]],rt[curr_task2]);
                        ft[curr_task2] = rt[curr_task2] + t_l_k[curr_task2][core2[curr_task2]];
                        core3[core2[curr_task2]] = ft[curr_task2];
                    }
                    //update ready1 and ready2
                    for(int a=0; a<n; a++){
                        if(taskGraph[curr_task2][a] == 1){
                            ready1[a] -= 1;
                        }
                    }
                    ready2[curr_task2] = 1;
                    if(tlist[core2[curr_task2]].size()>1){
                        for(int a=1; a<tlist[core2[curr_task2]].size(); a++){
                            if(tlist[core2[curr_task2]][a-1] == curr_task2){
                                ready2[tlist[core2[curr_task2]][a]] = 0;
                            }
                        }
                    }
                    for(int a=0; a<n; a++){
                        if(ready1[a] == 0 && ready2[a] == 0 && pushed[a] == 0){
                            s.push(a);
                            pushed[a] = 1;
                            //cout<<a<<" ";
                        }
                    }
                    //cout<<"s";
                }
                //cout<<endl;
                int current_t = ft[n-1];
                //int current_e = 0;
                float current_e = 0;
                for(int a=0; a<n; a++){
                    if(core2[a] == n_core){
                        current_e += 1.5;
                    }
                    else{
                        current_e += E_l[a][core2[a]];
                    }
                }
                //cout<<current_t<<" "<<i<<" "<<j<<endl;
                //判断当前choice的时间和能量
                if(current_t <= t_total && current_e < new_e){
                    less_t1 = 1;
                    new_n = i;
                    new_k = j;
                    new_index1 = index1;
                    new_index2 = index2;
                    new_t = current_t;
                    new_e = current_e;
                    //cout<<new_t<<endl;
                    for(int a=0; a<n; a++){
                        temp_core[a] = core2[a];
                        new_st[a] = rt[a];
                        new_ft[a] = ft[a];
                    }
                }
                if(current_t > t_total && current_t <= tmax && less_t1 == 0 && current_e < e_total && max_ratio < double((e_total - current_e) / (current_t - t_total))){
                    max_ratio = double((e_total - current_e) / (current_t - t_total));
                    //cout<<max_ratio<<endl;
                    less_t2 = 1;
                    new_n = i;
                    new_k = j;
                    new_index1 = index1;
                    new_index2 = index2;
                    new_t = current_t;
                    new_e = current_e;
                    //cout<<new_t<<endl;
                    for(int a=0; a<n; a++){
                        temp_core[a] = core2[a];
                        new_st[a] = rt[a];
                        new_ft[a] = ft[a];
                    }
                }

            }
        }
        if(less_t1 != 1 && less_t2 != 1){
            out = 1;
        }
        else{
            clist[core1[new_n]].erase(clist[core1[new_n]].begin()+new_index1);
            clist[new_k].insert(clist[new_k].begin()+new_index2,new_n);
            t_total = new_t;
            e_total = new_e;
            for(int a=0; a<n; a++){
                core1[a] = temp_core[a];
                st[a] = new_st[a];
                et[a] = new_ft[a];
            }
            if(less_t1 != 1 && less_t2 != 1){
                out = 1;
            }
            count += 1;
            cout<<count<<"th step "<<endl;
            cout<<"Operation: schedule Task "<<new_n+1<<" to Core "<<new_k+1<<endl;
            cout<<"Completion Time: "<<t_total<< "    Energy Consumption: "<<e_total<<endl;
        }
    }

    cout<<endl;
    cout<<"Best Schedule"<<endl;
    cout<<"Best Completion Time: "<<t_total<<"   Best Energy Consumption: "<<e_total<<endl;
    for(int i=0; i<clist.size(); i++){
        if(i == n_core){
            cout<<"Clould: ";
        }
        else{
            cout<<"Core"<<i+1<<": ";
        }
        for(int j=0; j<clist[i].size(); j++){
            cout<<clist[i][j]+1<<" ";
        }
        cout<<endl;
    }
    cout<<endl;
    cout<<"Schedule Details"<<endl;

    for(int i=0; i<clist.size(); i++){
        if(i == n_core){
            cout<<"Cloud: ";
        }
        else{
            cout<<"Core "<<i+1<<": ";
        }
        for(int j=0; j<clist[i].size(); j++){
            cout<<st[clist[i][j]]<<"-Task"<<clist[i][j]+1<<"-"<<et[clist[i][j]]<<" ";
        }
        cout<<endl;
    }
    cout<<"Completion Time: "<<t_total<<"   Energy Consumption: "<<e_total<<endl;
}


int main() {
    //running time of tasks in cores
//    int t1[][core_n]={{9,7,5},
//                      {8,6,5},
//                      {6,5,4,},
//                      {7,5,3},
//                      {5,4,2},
//                      {7,6,4},
//                      {8,5,3},
//                      {6,4,2},
//                      {5,3,2},
//                      {7,4,2},
//                      {7,5,4},
//                      {6,4,2},
//                      {8,5,3},
//                      {9,6,3},
//                      {7,5,3},
//                      {5,4,3},
//                      {9,7,5},
//                      {8,6,5},
//                      {7,6,3},
//                      {8,6,4}};
//
//    int t_succ[][20]={{0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//                      {0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
//                      {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0},
//                      {0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0},
//                      {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
//                      {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

    int t_l_k[][n_core]={
            {9,7,5},
            {8,6,5},
            {6,5,4},
            {7,5,3},
            {5,4,2},
            {7,6,4},
            {8,5,3},
            {6,4,2},
            {5,3,2},
            {7,4,2}};
    //adjacency matrix
    int taskGraph[][n_task]={
            {0,1,1,1,1,1,0,0,0,0},
            {0,0,0,0,0,0,0,1,1,0},
            {0,0,0,0,0,0,1,0,0,0},
            {0,0,0,0,0,0,0,1,1,0},
            {0,0,0,0,0,0,0,0,1,0},
            {0,0,0,0,0,0,0,1,0,0},
            {0,0,0,0,0,0,0,0,0,1},
            {0,0,0,0,0,0,0,0,0,1},
            {0,0,0,0,0,0,0,0,0,1},
            {0,0,0,0,0,0,0,0,0,0}};

    int i, j;
    //n number of graphs, and k number of cores
    int t_s=3, t_c=1, t_r=1;
    int t_re =t_s+ t_c+ t_r;
    int C[n_task];  //where to execute task i ?   0 in local, and 1 in cloud
    float W[n_task];
    float Priority[n_task];
    int PriOrder[n_task];
    int core_time[n_core+1]={0}; //  0-2 是local core； 3 是cloud ws;
    int rt_l[n_task], ft_l[n_task], ft_wr[n_task],  ft_ws[n_task],
        rt_ws[n_task],  rt_c[n_task],  ft_c[n_task], ft[n_task], task_loc[n_task];
    vector<vector<int>> core_list(n_core+1);
    int st[n_task];
    float pk[n_core]={1,2,4}; //local core 的单位能耗
    float e_c = float(0.5*t_s);  //cloud上的能耗
    float e_total=0;
    int tmin ;
    int tmax ;
    int E_l[n_task][n_core];

    //############# input1 #############
    cout<<"############# input1 #############";
    for(int i=0; i<n_task; i++){
        rt_l[i]=0;
        ft_l[i]=0;
        ft_wr[i]=0;
        ft_ws[i]=0;
        rt_ws[i]=0;
        rt_c[i]=0;
        ft_c[i]=0;
        ft[i]=0;
        task_loc[i]=0;
    }

    Primary_assignment(t_l_k, n_task, n_core, t_re, C);
    Task_prioritizing(t_l_k, taskGraph, Priority, PriOrder, W, C, n_core, n_task, t_re);
    Unit_selection(n_task, n_core, PriOrder, t_l_k, taskGraph, core_time,
                   rt_l, ft_l, ft_wr, ft_ws, rt_ws, rt_c, ft_c, ft, t_s, t_c, t_r, task_loc);

    //calculate start time of each task
    for(i=n_task-1; i>=0; i--){
        int task = PriOrder[i];
        st[task] = max(rt_l[task], rt_ws[task]);
        //cout<<st[task]<<" ";
    }
//    for(int i=0; i<n_core; i++){
//        st[i] = max(rt_l[i],rt_ws[i]);
//    }

    //task schedule visualization
    for(i=0; i<n_task; i++){
        core_list[task_loc[i]].push_back(i);
    }
    cout<<"Initial Scheduling (st - Task Number - ft)"<<"\n";
    for(i=0; i<core_list.size(); i++){
        if(i==core_list.size()-1){
            cout<<"cloud: ";
        }
        else{
            cout<<"core "<<i+1<<": ";
        }
        for(j=0; j<core_list[i].size(); j++){
            cout<<st[core_list[i][j]]<<"-task"<<core_list[i][j]+1<<"-"<<ft[core_list[i][j]]<<" ";
        }
        cout<<"\n";
    }
    cout<<"initial schedule time:"<<ft[PriOrder[0]]<<"\n";

    // energy consumption
    for(i=0; i<n_core+1; i++){
        for(j=0; j<core_list[i].size(); j++){
            if(i<n_core){   //local
                e_total = e_total + (pk[i]*float(t_l_k[ core_list[i][j] ][i]));
            }
            else{
                e_total = e_total + e_c;
            }
        }
    }
    cout<<"initial schedule energy:"<<e_total<<"\n\n";

    tmin = ft[n_task-1];
    tmax = ft[n_task-1]*1.5;

    for(int i=0; i<10; i++){
        for(int j=0; j<3; j++){
            E_l[i][j] = pk[j] * t_l_k[i][j];
        }
    }
    //Kernel_algorithm(core_list,t_l_k,taskGraph, task_loc, ft, st);
    kernel(core_list, t_l_k, taskGraph, task_loc, tmax, tmin,e_total,
           n_task, n_core, st, ft, e_c, E_l);




    //############# input2 #############
    cout<<"\n############# input2 #############";
    int taskGraph2[][n_task]={
            {0,1,1,1,1,1,0,0,0,0},
            {0,0,0,0,0,0,0,0,1,0},
            {0,0,0,0,0,0,1,0,0,0},
            {0,0,0,0,0,0,0,1,0,0},  //去掉4-9， 2-8
            {0,0,0,0,0,0,0,0,1,0},
            {0,0,0,0,0,0,0,1,0,0},
            {0,0,0,0,0,0,0,0,0,1},
            {0,0,0,0,0,0,0,0,0,1},
            {0,0,0,0,0,0,0,0,0,1},
            {0,0,0,0,0,0,0,0,0,0}};




    for(int i=0; i<n_task; i++){
        rt_l[i]=0;
        ft_l[i]=0;
        ft_wr[i]=0;
        ft_ws[i]=0;
        rt_ws[i]=0;
        rt_c[i]=0;
        ft_c[i]=0;
        ft[i]=0;
        task_loc[i]=0;
        Priority[i]=0;
        PriOrder[i]=0;
        core_time[i]=0;
        W[i]=0;
        C[i]=0;
        st[i]=0;
    }
    e_total=0;

    Primary_assignment(t_l_k, n_task, n_core, t_re, C);
    Task_prioritizing(t_l_k, taskGraph2, Priority, PriOrder, W, C, n_core, n_task, t_re);
    Unit_selection(n_task, n_core, PriOrder, t_l_k, taskGraph2, core_time,
                   rt_l, ft_l, ft_wr, ft_ws, rt_ws, rt_c, ft_c, ft, t_s, t_c, t_r, task_loc);

    //calculate start time of each task
    for(i=n_task-1; i>=0; i--){
        int task = PriOrder[i];
        st[task] = max(rt_l[task], rt_ws[task]);
        //cout<<st[task]<<" ";
    }

    //task schedule visualization
    vector<vector<int>> core_list2(n_core+1);
    for(i=0; i<n_task; i++){
        core_list2[task_loc[i]].push_back(i);
    }


    cout<<"Initial Scheduling (st - Task Number - ft)"<<"\n";
    for(i=0; i<core_list2.size(); i++){
        if(i==core_list2.size()-1){
            cout<<"cloud: ";
        }
        else{
            cout<<"core "<<i+1<<": ";
        }
        for(j=0; j<core_list2[i].size(); j++){
            cout<<st[core_list2[i][j]]<<"-task"<<core_list2[i][j]+1<<"-"<<ft[core_list2[i][j]]<<" ";
        }
        cout<<"\n";
    }
    cout<<"initial schedule time:"<<ft[PriOrder[0]]<<"\n";

    // energy consumption
    for(i=0; i<n_core+1; i++){
        for(j=0; j<core_list2[i].size(); j++){
            if(i<n_core){   //local
                e_total = e_total + (pk[i]*float(t_l_k[ core_list2[i][j] ][i]));
            }
            else{
                e_total = e_total + e_c;
            }
        }
    }
    cout<<"initial schedule energy:"<<e_total<<"\n\n";


    tmin = ft[n_task-1];
    tmax = ft[n_task-1]*1.5;
    for(int i=0; i<10; i++){
        for(int j=0; j<3; j++){
            E_l[i][j] = pk[j] * t_l_k[i][j];
        }
    }
    //Kernel_algorithm(core_list,t_l_k,taskGraph, task_loc, ft, st);
    kernel(core_list2, t_l_k, taskGraph2, task_loc, tmax, tmin,e_total,
           n_task, n_core, st, ft, e_c, E_l);


}
