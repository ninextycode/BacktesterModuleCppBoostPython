#include "stdafx.h"
#include "Backtester.h"


MarketDepth::MarketDepth(std::string ticker) {
	this->totalDepth = MarketDepthData();
	this->ticker = ticker;
}


MarketDepth::~MarketDepth() {
}

std::vector<OrderChange> MarketDepth::addOrder(Order order) {
	currentChanges.clear();
	tryMatchOrder(order);
	if (order.volume != 0) {
		auto it_whereOrdersWithSuchPrice = this->totalDepth.find(order.price);
		if (it_whereOrdersWithSuchPrice == this->totalDepth.end()) {
			(this->totalDepth)[order.price] = { order };
		} else {
			it_whereOrdersWithSuchPrice->second.push_front(order);
		}
	}
	return currentChanges;
}


void MarketDepth::tryMatchOrder(Order& order) {
	if (order.volume > 0) {
		tryMatchBuyOrder(order);
	} else {
		tryMatchSellOrder(order);
	}
}

void MarketDepth::tryMatchBuyOrder(Order& order) {
	auto prices = this->getUnsortedPrices();
	std::sort(prices.begin(), prices.end());

	for (int price : prices) {
		if (order.price >= price && (totalDepth)[price].begin()->volume < 0) {
			tryMatchOrderWithExactPriceOrders(order, price);
			if (order.volume == 0) 
				break;
		}
	}
}

void MarketDepth::tryMatchSellOrder(Order& order) {
	auto prices = this->getUnsortedPrices();
	std::sort(prices.begin(), prices.end(), [&](auto a, auto b) {return a > b; });

	for (int price : prices) {
		if (order.price <= price && (totalDepth)[price].begin()->volume > 0) {
			tryMatchOrderWithExactPriceOrders(order, price);
			if (order.volume == 0) break;
		}
	}
}

std::vector<int> MarketDepth::getUnsortedPrices() {
	std::vector<int> prices;
	prices.reserve(this->totalDepth.size());
	for (auto pair : this->totalDepth) {
		prices.push_back(pair.first);
	}
	return prices;
}

void MarketDepth::tryMatchOrderWithExactPriceOrders(Order& order, int price) {
	auto &ordersList = (totalDepth)[price];
	for (auto it = ordersList.begin(); it != ordersList.end(); /* "it" is incrementing */) {
		if (abs(order.volume) >= abs(it->volume)) { //Vanishing of order already being inside depth

			this->currentChanges.
					push_back(OrderChange::ChangesOfOrderVanishing(*it, ChangeReason::Match));

			Order oldOrder = order;
			order.volume += it->volume;

			this->currentChanges.
				push_back(OrderChange::MakeChangesByComparison(oldOrder, order, ChangeReason::Match));
			
			
			it = ordersList.erase(it); // <-- here incrementation
		} else {  //Vanishing of newly recieved order

			this->currentChanges.
				push_back(OrderChange::ChangesOfOrderVanishing(order, ChangeReason::Match));
			
			Order oldOrder = *it;
			it->volume += order.volume;
			order.volume = 0;

			this->currentChanges.
				push_back(OrderChange::MakeChangesByComparison(oldOrder, *it, ChangeReason::Match));
					
			break;
		}
	}
	if (ordersList.size() == 0) {
		totalDepth.erase(price);
	}
}


std::vector<OrderChange>  MarketDepth::changeOrder(Order order) {
	currentChanges.clear();
	removeOrdersOfSuchTraderWithSuchPrice(order);
	if (order.volume != 0) {
		auto it_whereOrdersWithSuchPrice = this->totalDepth.find(order.price);
		if (it_whereOrdersWithSuchPrice == this->totalDepth.end()) {
			(this->totalDepth)[order.price] = { order };
		} else {
			it_whereOrdersWithSuchPrice->second.push_front(order);
		}
	}
	return currentChanges;
}

void MarketDepth::removeOrdersOfSuchTraderWithSuchPrice(Order newOrder) {
	auto it_whereOrdersWithSuchPrice = this->totalDepth.find(newOrder.price);
	if (it_whereOrdersWithSuchPrice != this->totalDepth.end()) {
		std::list<Order>& ordersList = it_whereOrdersWithSuchPrice->second;
		for (auto it = ordersList.begin(); it != ordersList.end(); ) {
			if (it->trader_identifier == newOrder.trader_identifier) {

				this->currentChanges.
					push_back(OrderChange::ChangesOfOrderVanishing(*it, ChangeReason::Cancellation));

				it = ordersList.erase(it);

			} else {
				it++;
			}
		}
	}
}

AnonimousMaketDepth  MarketDepth::getAnonimousDepth(int depthLength) {
	return std::vector<std::tuple<int, int>>{ 
		std::tuple<int, int>(1,2),
		std::tuple<int, int>(3,4)
	};
}

MarketDepthData const &  MarketDepth::getInternalDepth() {
	return this->totalDepth;
}
