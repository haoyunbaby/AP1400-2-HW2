#include "client.h"

#include <limits>
#include <random>
#include <string>

#include "crypto.h"
using namespace std;

Client::Client(string id, const Server &server) : id(id), server(&server) {
  crypto::generate_key(public_key, private_key);
}

string Client::get_id() const { return id; }
string Client::get_publickey() const { return public_key; }

double Client::get_wallet() const { return server->get_wallet(id); }

string Client::sign(string txt) const {
  string signature = crypto::signMessage(private_key, txt);
  return signature;
}
bool Client::transfer_money(string receiver, double value) {
  string trx;
  trx += get_id();
  trx += '-';
  trx += receiver;
  trx += '-';
  trx += to_string(value);
  string signature = sign(trx);
  bool f = server->add_pending_trx(trx, signature);
  return f;
}
size_t Client::generate_nonce() {
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<size_t> dis(0, numeric_limits<size_t>::max());
  size_t nonce = dis(gen);
  return nonce;
}
