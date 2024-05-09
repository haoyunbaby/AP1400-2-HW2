#include "server.h"

#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "crypto.h"

using namespace std;

vector<string> pending_trxs;
void show_pending_transactions() {
  cout << string(20, '*') << endl;
  for (const auto &trx : pending_trxs) {
    cout << trx << endl;
  }
  cout << string(20, '*') << endl;
}

void show_wallets(const Server &server) {
  cout << string(20, '*') << endl;
  for (const auto &client : server.clients) {
    cout << client.first->get_id() << " : " << client.second << endl;
  }
  cout << string(20, '*') << endl;
}

Server::Server() {}
shared_ptr<Client> Server::add_client(string id) {
  if (clients.find(get_client(id)) != clients.end()) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis(0, 9);
    for (int i = 0; i < 4; i++) {
      int rnd = dis(gen);
      id += to_string(rnd);
    }
  }
  shared_ptr<Client> spc = make_shared<Client>(id, *this);
  clients[spc] = 5;
  return spc;
}
shared_ptr<Client> Server::get_client(string id) const {
  for (const auto &p : clients) {
    if (p.first->get_id() == id) {
      return p.first;
    }
  }
  return nullptr;
}
double Server::get_wallet(string id) const {
  for (const auto &p : clients) {
    if (p.first->get_id() == id) {
      return p.second;
    }
  }
  return 0;
}
bool Server::parse_trx(string trx, string &sender, string &receiver,
                       double &value) {
  vector<string> tokens;
  stringstream ss(trx);
  string token;
  while (getline(ss, token, '-')) {
    tokens.push_back(token);
  }
  if (tokens.size() != 3) {
    throw runtime_error("parse_trx");
  }
  sender = tokens[0];
  receiver = tokens[1];
  value = stod(tokens[2]);
  return true;
}
bool Server::add_pending_trx(string trx, string signature) const {
  string sender, receiver;
  double value;
  parse_trx(trx, sender, receiver, value);
  auto p_sender = get_client(sender);
  bool authentic =
      crypto::verifySignature(p_sender->get_publickey(), trx, signature);
  if (!get_client(sender)) {
    return false;
  }
  if (!get_client(receiver)) {
    return false;
  }
  if (!authentic) {
    return false;
  }
  if (get_wallet(sender) < value) {
    return false;
  }
  pending_trxs.push_back(trx);
  return true;
}
size_t Server::mine() {
  // std::string hash{crypto::sha256("hi")}
  string mempool;
  for (auto &trx : pending_trxs) {
    mempool += trx;
  }
  while (1) {
    for (auto &p : clients) {
      string t = mempool;
      // t += to_string(p.first->generate_nonce());
      size_t nonce = p.first->generate_nonce();
      t += to_string(nonce);
      string hash = crypto::sha256(t);
      if (hash.substr(0, 10).find("000") != string::npos) {
        p.second += 6.25;
        for (const auto &trx : pending_trxs) {
          string sender, receiver;
          double value;
          parse_trx(trx, sender, receiver, value);
          auto p_sender = get_client(sender), p_receiver = get_client(receiver);
          clients[p_sender] -= value;
          clients[p_receiver] += value;
        }
        pending_trxs.clear();
        cout << p.first->get_id() << '\n';
        return nonce;
      }
    }
  }
  return 0;
}
