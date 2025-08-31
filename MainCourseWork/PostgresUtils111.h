#pragma once

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>

#include <iostream>
#include <string>
#include <vector>
#include <set>


namespace dbo = Wt::Dbo;


namespace ak
{
	class Host;
	class Word;



	class Shop;
	class Book;
	class Stock;
	class Sale;

	class Publisher {
	public:
		Publisher() {}
		explicit Publisher(std::string name) : name(name) {}

		std::string name{};
		dbo::collection<dbo::ptr<Book>> books{};

		template<class Action>
		void persist(Action& a)
		{
			dbo::field(a, name, "name");
			dbo::hasMany(a, books, dbo::ManyToOne, "publisher");
		}
	};

	class Shop {
	public:
		Shop() {}
		explicit Shop(std::string name) : name(name) {}

		std::string name{};
		dbo::collection<dbo::ptr<Stock>> stocks{};

		template<class Action>
		void persist(Action& a)
		{
			dbo::field(a, name, "name");
			dbo::hasMany(a, stocks, dbo::ManyToOne, "shop");
		}
	};

	class Book {
	public:
		Book() {}
		explicit Book(std::string title) : title(title) {}

		std::string title{};
		dbo::ptr<Publisher> publisher{};
		dbo::collection<dbo::ptr<Stock>> stocks{};

		template<class Action>
		void persist(Action& a)
		{
			dbo::field(a, title, "title");
			dbo::belongsTo(a, publisher, "publisher");
			dbo::hasMany(a, stocks, dbo::ManyToOne, "book");
		}
	};

	class Stock {
	public:
		Stock() {}
		explicit Stock(int count) : count(count) {}

		int count{};
		dbo::ptr<Book> book{};
		dbo::ptr<Shop> shop{};
		dbo::collection<dbo::ptr<Sale>> sales{};

		template<class Action>
		void persist(Action& a)
		{
			dbo::field(a, count, "count");
			dbo::belongsTo(a, book, "book");
			dbo::belongsTo(a, shop, "shop");
			dbo::hasMany(a, sales, dbo::ManyToOne, "stock");
		}
	};

	class Sale {
	public:
		Sale() {}
		explicit Sale(int price, std::string date_sale, int count) :
			price(price), date_sale(date_sale), count(count) {
		}

		int price{};
		std::string date_sale{};
		int count{};
		dbo::ptr<Stock> stock{};

		template<class Action>
		void persist(Action& a)
		{
			dbo::field(a, price, "price");
			dbo::field(a, date_sale, "date_sale");
			dbo::field(a, count, "count");
			dbo::belongsTo(a, stock, "stock");
		}
	};


	template <typename T>
	void addEntry(dbo::Session& session, T* pObj)
	{
		dbo::Transaction transaction{ session };

		std::unique_ptr<T> upObj{ pObj };
		dbo::ptr<T> ptr = session.add(std::move(upObj));

		transaction.commit();
	}

	template <typename T, typename T2>
	void addEntry(dbo::Session& session, T* pObj, std::string str)
	{
	}

	template <typename T, typename T2>
	void addEntry(dbo::Session& session, T* pObj, int id)
	{
	}

	template <typename T, typename T2, typename T3>
	void addEntry(dbo::Session& session, T* pObj, std::string str1, std::string str2)
	{
	}

	template <>
	void addEntry<Book, Publisher>(dbo::Session& session, Book* pObj, std::string str)
	{
		dbo::Transaction transaction{ session };

		std::unique_ptr<Book> upObj{ pObj };
		dbo::ptr<Book> ptr = session.add(std::move(upObj));

		dbo::ptr<Publisher> publisher = session.find<Publisher>().where("name = ?").bind(str);
		ptr.modify()->publisher = publisher;

		transaction.commit();
	}

	template <>
	void addEntry<Stock, Book, Shop>(dbo::Session& session, Stock* pObj,
		std::string str1, std::string str2)
	{
		dbo::Transaction transaction{ session };

		std::unique_ptr<Stock> upObj{ pObj };
		dbo::ptr<Stock> ptr = session.add(std::move(upObj));

		dbo::ptr<Book> book = session.find<Book>().where("title = ?").bind(str1);
		dbo::ptr<Shop> shop = session.find<Shop>().where("name = ?").bind(str2);
		ptr.modify()->book = book;
		ptr.modify()->shop = shop;

		transaction.commit();
	}

	template <>
	void addEntry<Sale, Stock>(dbo::Session& session, Sale* pObj, int id)
	{
		dbo::Transaction transaction{ session };

		std::unique_ptr<Sale> upObj{ pObj };
		dbo::ptr<Sale> ptr = session.add(std::move(upObj));

		dbo::ptr<Stock> stock = session.find<Stock>().where("id = ?").bind(id);
		ptr.modify()->stock = stock;

		transaction.commit();
	}

	template <typename T>
	std::set<std::string> getShops(dbo::Session& session, std::string str)
	{
	}

	template <>
	std::set<std::string> getShops<Publisher>(dbo::Session& session, std::string str)
	{
		std::set<std::string> ret{};

		dbo::Transaction transaction{ session };

		dbo::ptr<Publisher> publisher = session.find<Publisher>().where("name = ?").bind(str);

		dbo::collection<dbo::ptr<Book>> books = publisher->books;

		for (const auto& book : books)
		{
			dbo::collection<dbo::ptr<Stock>> stocks = book->stocks;

			for (const auto& stock : stocks)
			{
				ret.emplace(stock->shop->name);
			}
		}

		transaction.commit();
		return ret;
	}
}
