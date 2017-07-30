#include <time.h>
#include <pthread.h>
#include <iostream>
#include "echoMsg.pb.h"
#include "easy_reactor.h"

using namespace std;
using namespace echo;

struct testQPS
{
    testQPS(): lstTs(0), succ(0) {}
    long lstTs;
    int succ;
};

void buz(const char* data, uint32_t len, int cmdid, net_commu* commu, void* usr_data)
{
    testQPS* qps = (testQPS*)usr_data;

    EchoString req, rsp;

    if (rsp.ParseFromArray(data, len))
    {
        qps->succ++;
    }
    long curTs = time(NULL);
    if (curTs - qps->lstTs >= 1)
    {
        cout << qps->succ << endl;
        qps->lstTs = curTs;
        qps->succ = 0;
    }

    req.set_id(rsp.id() + 1);
    req.set_content(rsp.content());

    string reqStr;
    req.SerializeToString(&reqStr);
    commu->send_data(reqStr.c_str(), reqStr.size(), cmdid);
}

void* domain(void* args)
{
    event_loop loop;
    tcp_client client(&loop, "127.0.0.1", 12315);

    testQPS qps;
    client.add_msg_cb(1, buz, (void*)&qps);

    EchoString req;
    req.set_id(100);
    req.set_content("My name is LeechanX, I miss Helen Liang very much");
    string reqStr;
    req.SerializeToString(&reqStr);
    client.send_data(reqStr.c_str(), reqStr.size(), 1);

    loop.process_evs();
    return NULL;
}

int main(int argc, char** argv)
{
    int threadNum = atoi(argv[1]);
    pthread_t *tids;
    tids = new pthread_t[threadNum];
    for (int i = 0;i < threadNum; ++i)
        pthread_create(&tids[i], NULL, domain, NULL);
    for (int i = 0;i < threadNum; ++i)
        pthread_join(tids[i], NULL);
    return 0;
}