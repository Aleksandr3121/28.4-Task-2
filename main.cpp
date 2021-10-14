#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <string>
#include <random>
#include <functional>

using namespace std;

bool CheckingInputStream(istream& stream) {
    if (stream)return true;
    stream.clear();
    stream.ignore(32756, '\n');
    cerr << "Error of input" << endl;
    return false;
}

struct Train {
    string id;
    chrono::steady_clock::time_point start;
    chrono::seconds travelTime;
};

class ManagerTrains {
public:
    explicit ManagerTrains(vector<Train> inTrains) : trains(move(inTrains)) {}

    void Start() {
        //рандомные числа для имитации разного времени отбытия поездов
        random_device rd;
        mt19937 mt(rd());
        vector<thread> threads;
        for (int i = 0; i < 3; ++i) {
            threads.push_back(thread(&ManagerTrains::StartOneTrain, this, ref(trains[i]), mt() % 10));
        }
        for (int i = 0; i < 3; ++i)threads[i].join();
    }

private:
    void StartOneTrain(Train& train, int timeStart) {
        this_thread::sleep_for(chrono::seconds{timeStart});
        train.start = chrono::steady_clock::now();
        //В документации про sleep_for сказано:
        //This function may block for longer than due to scheduling or resource contention delays. sleep_duration
        //Насколько может быть длительной задержка? Корректно ли здесь использовать эту функцию?
        this_thread::sleep_for(train.travelTime);

        //Есть ли смысл здесь блокировать консольный вывод?
        //Или cout сам разберется от какого потока первым выводить сообщение?
        mCout.lock();
        cout << train.id << " is waiting for an empty seat" << endl;
        mCout.unlock();
        mTrainStation.lock();
        mCout.lock();
        cout << train.id << " arrived at the station" << endl;
        mCout.unlock();
        string command;
        while (cin >> command && command != "depart") {
            cerr << "Unknown command" << endl;
        }
        mTrainStation.unlock();
        mCout.lock();
        cout << train.id << " was deported from the station" << endl;
        mCout.unlock();
    }

    vector<Train> trains;
    mutex mCout;
    mutex mTrainStation;
};

ManagerTrains CreateManagerTrains() {
    vector<Train> trains(3);
    for (int i = 0; i < 3; ++i) {
        cout << "Enter id of train " << i + 1 << ' ' << endl;
        cin >> trains[i].id;
        cout << "Enter travel time for train " << trains[i].id << endl;
        int dur;
        do {
            cin >> dur;
        } while (!CheckingInputStream(cin) || dur <= 0);
        trains[i].travelTime = chrono::seconds{dur};
    }
    return ManagerTrains(move(trains));
}

int main() {
    ManagerTrains managerTrains = CreateManagerTrains();
    managerTrains.Start();
    return 0;
}
