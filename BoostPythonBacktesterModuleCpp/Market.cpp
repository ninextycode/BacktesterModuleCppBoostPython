#include "stdafx.h"
#include "Backtester.h"


SngleTraderMarket::SngleTraderMarket() {
}


SngleTraderMarket::~SngleTraderMarket() {
}

void SngleTraderMarket::setTrader(Trader * trader) {
	this->trader = trader;
}

void SngleTraderMarket::loadHistoryData(std::string path, std::vector<std::string> tickers) {
	if (path.at(path.length - 1) != '/') {
		path += "/";
	}
	for (auto ticker : tickers) {
		this->tickers.push_back(ticker);
		readFile(path + ticker + ".csv", ticker);
	}
}

void SngleTraderMarket::readFile(std::string path, std::string ticker) {
	std::ifstream input(path);
	if (!input.is_open()) {
		throw std::invalid_argument("Wrong path");
	}

	while (!input.eof()) {
		std::string s;
		std::getline(input, s);

		std::string datetime = s.substr(0, s.find_first_of(","));
		s = s.substr(s.find_first_of(",") + 1);
		std::string open = s.substr(0, s.find_first_of(","));
		s = s.substr(s.find_first_of(",") + 1);
		std::string high = s.substr(0, s.find_first_of(","));
		s = s.substr(s.find_first_of(",") + 1);
		std::string low = s.substr(0, s.find_first_of(","));
		s = s.substr(s.find_first_of(",") + 1);
		std::string close = s.substr(0, s.find_first_of(","));
		s = s.substr(s.find_first_of(",") + 1);
		std::string volume = s.substr(0, s.find_first_of(","));
		s = s.substr(s.find_first_of(",") + 1);

		if (datetime.size() * open.size() * high.size() * low.size() * close.size() == 0) {
			continue;
		}

		Candel c;
		c.open = stoi(open);
		c.high = stoi(high);
		c.low = stoi(low);
		c.close = stoi(close);
		c.volume = volume.size() == 0 ? 0 : stoi(volume);
		c.datetime = boost::posix_time::time_from_string(datetime);
		historyData[ticker].push_back(c);
	}
	if (numberOfTics == -1) {
		numberOfTics = historyData[ticker].size();
	} else {
		numberOfTics = min(historyData[ticker].size(), numberOfTics);
	}
}

void SngleTraderMarket::runFullTest() {
	for (currentTick = 0; currentTick < numberOfTics; currentTick++) {
		tick();
	}
}

void SngleTraderMarket::addOrder(Order order) {
	ordersToAdd.push_back(order);
}

void SngleTraderMarket::changeOrder(Order order) {
	//depths[ticker].cancelOrder(trades_identifier, );
}

void SngleTraderMarket::tick() {
	this->updateDepths();
	this->addAndCleanOrders();
	this->sendAndCleanTickData();
	this->sendAndCleanCandles();
}

void SngleTraderMarket::updateDepths() {
	for (auto ticker : tickers) {
		depths[ticker].clearHistoryOrders();
		auto currentCandle = historyData[ticker][currentTick];
		auto orders = OrdersFromCandelBuilder::ordersFromCandel(currentCandle, ticker);
		for (Order order : orders) {
			auto changes = depths[ticker].addOrder(order);
			changesToSend.insert(changesToSend.end(), changes.begin(), changes.end());
		}
	}
}

void SngleTraderMarket::addAndCleanOrders() {
	for (Order order : this->ordersToAdd) {
		auto desiderDepth = this->depths.find(order.ticker);
		desiderDepth->second.addOrder(order);		
	}
	ordersToAdd.clear();
}

void SngleTraderMarket::sendAndCleanTickData() {
	this->trader->recieveTickData(changesToSend, anonimousDepths);
	changesToSend.clear();
}

void SngleTraderMarket::sendAndCleanCandles() {
	this->trader->recieveCandels(candelsToSend);
	candelsToSend.clear();
}

void loadHistoryData(std::string path) {
}
