// Michelle 的学生会工作 (ACMOJ 1346)
// Implements a student management system per spec.

#include <bits/stdc++.h>
using namespace std;

struct Student {
    string name;
    char gender; // 'M' or 'F'
    int cls;     // 1..20
    int score[9];
    long long avg; // floor of average
    bool has_avg;  // whether avg has been computed (after START or after updates)
};

static inline long long calc_avg(const int sc[9]) {
    long long s = 0;
    for (int i = 0; i < 9; ++i) s += sc[i];
    return s / 9; // floor
}

struct RankKey {
    long long avg;
    array<int,9> sc;
    string name;
};

struct RankCmp {
    bool operator()(const RankKey &a, const RankKey &b) const {
        if (a.avg != b.avg) return a.avg > b.avg; // higher avg first
        for (int i = 0; i < 9; ++i) {
            if (a.sc[i] != b.sc[i]) return a.sc[i] > b.sc[i];
        }
        if (a.name != b.name) return a.name < b.name; // lexicographically smaller first
        return false; // equal
    }
};

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    unordered_map<string, Student> mp; // name -> student
    bool started = false;

    // ranking structure: store keys only
    set<RankKey, RankCmp> ranking;
    // name -> iterator in ranking set for quick erase/update
    unordered_map<string, set<RankKey, RankCmp>::iterator> itpos;
    // name -> current rank (valid after a ranking refresh)
    unordered_map<string, int> rank_of;
    bool rank_valid = false; // invalidated on changes until FLUSH or START sorts

    string cmd;
    while (cin >> cmd) {
        if (cmd == "ADD") {
            string name; char gender; int cls; int sc[9];
            cin >> name >> gender >> cls;
            for (int i = 0; i < 9; ++i) cin >> sc[i];
            if (started) {
                cout << "[Error]Cannot add student now." << '\n';
                continue;
            }
            if (mp.find(name) != mp.end()) {
                cout << "[Error]Add failed." << '\n';
                continue;
            }
            Student st; st.name = name; st.gender = gender; st.cls = cls; st.has_avg = false; st.avg = 0;
            for (int i = 0; i < 9; ++i) st.score[i] = sc[i];
            mp.emplace(name, std::move(st));
            // before START, ranking not maintained. ranks invalid.
            rank_valid = false;
        } else if (cmd == "START") {
            if (!started) {
                started = true;
                ranking.clear();
                itpos.clear();
                rank_of.clear();
                for (auto &kv : mp) {
                    Student &st = kv.second;
                    st.avg = calc_avg(st.score);
                    st.has_avg = true;
                    RankKey key; key.avg = st.avg; key.name = st.name; for(int i=0;i<9;++i) key.sc[i]=st.score[i];
                    auto it = ranking.insert(key).first;
                    itpos[st.name] = it;
                }
                // compute initial ranks
                int r = 1;
                for (auto it = ranking.begin(); it != ranking.end(); ++it, ++r) {
                    rank_of[it->name] = r;
                }
                rank_valid = true;
            }
            // If START appears only once as guaranteed, subsequent START ignored.
        } else if (cmd == "UPDATE") {
            string name; int code, score;
            cin >> name >> code >> score;
            auto itmp = mp.find(name);
            if (itmp == mp.end()) {
                cout << "[Error]Update failed." << '\n';
                continue;
            }
            Student &st = itmp->second;
            // if started and already in ranking, update there as well (maintain correct order as per tips)
            if (started) {
                // erase old key
                auto itf = itpos.find(name);
                if (itf != itpos.end()) {
                    ranking.erase(itf->second);
                    itpos.erase(itf);
                }
            }
            st.score[code] = score;
            if (started) {
                st.avg = calc_avg(st.score);
                st.has_avg = true;
                RankKey key; key.avg = st.avg; key.name = st.name; for(int i=0;i<9;++i) key.sc[i]=st.score[i];
                auto itn = ranking.insert(key).first;
                itpos[name] = itn;
                // ranking changed; ranks need recompute until FLUSH
                rank_valid = false;
            }
        } else if (cmd == "FLUSH") {
            if (started) {
                // recompute ranks based on current ranking order
                int r = 1;
                for (auto it = ranking.begin(); it != ranking.end(); ++it, ++r) {
                    rank_of[it->name] = r;
                }
                rank_valid = true;
            }
        } else if (cmd == "PRINTLIST") {
            if (started) {
                // print list using last flushed ranks order (stable until next FLUSH)
                // Build a vector of names sorted by rank_of ascending
                vector<pair<int,string>> order;
                order.reserve(rank_of.size());
                for (const auto &kv : rank_of) order.emplace_back(kv.second, kv.first);
                sort(order.begin(), order.end());
                for (const auto &pr : order) {
                    const string &name = pr.second;
                    const Student &st = mp[name];
                    cout << pr.first << ' ' << st.name << ' ' << (st.gender=='M'?"male":"female") << ' ' << st.cls << ' ' << st.avg << '\n';
                }
            }
        } else if (cmd == "QUERY") {
            string name; cin >> name;
            auto itmp = mp.find(name);
            if (itmp == mp.end()) {
                cout << "[Error]Query failed." << '\n';
                continue;
            }
            if (!started) {
                // before START, there is no ranking
                cout << "[Error]Query failed." << '\n';
                continue;
            }
            if (!rank_valid) {
                // ranking not flushed since last updates; ranking position remains as before
                // But we still need to output the last known ranking
                // If never flushed since START, rank_of should have initial ranks
            }
            auto itr = rank_of.find(name);
            if (itr == rank_of.end()) {
                // should not happen if started and flushed at least once at START
                cout << "[Error]Query failed." << '\n';
            } else {
                cout << "STUDENT " << name << " NOW AT RANKING " << itr->second << '\n';
            }
        } else if (cmd == "END") {
            break;
        }
    }

    return 0;
}
