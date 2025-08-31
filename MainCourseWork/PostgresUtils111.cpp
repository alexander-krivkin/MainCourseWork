


int main()
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	try
	{
		std::string connectionString =
			"host=localhost "
			"port=5432 "
			"dbname=test "
			"user=postgres "
			"password=Veryspecial1335!";
		auto postgres = std::make_unique<Wt::Dbo::backend::Postgres>(connectionString);
		dbo::Session session;
		session.setConnection(std::move(postgres));

		session.mapClass<Publisher>("publisher");
		session.mapClass<Shop>("shop");
		session.mapClass<Book>("book");
		session.mapClass<Stock>("stock");
		session.mapClass<Sale>("sale");

		session.dropTables();
		session.createTables();

		addEntry<Publisher>(session, new Publisher{ "Lee" });
		addEntry<Publisher>(session, new Publisher{ "Orwell" });
		addEntry<Publisher>(session, new Publisher{ "Fitzgerald" });
		addEntry<Publisher>(session, new Publisher{ "Salinger" });
		addEntry<Publisher>(session, new Publisher{ "Auste" });

		addEntry<Book, Publisher>(session, new Book{ "1984" }, "Orwell");
		addEntry<Book, Publisher>(session, new Book{ "To Kill a Mockingbird" }, "Lee");
		addEntry<Book, Publisher>(session, new Book{ "Animal Farm" }, "Orwell");
		addEntry<Book, Publisher>(session, new Book{ "The Great Gatsby" }, "Fitzgerald");
		addEntry<Book, Publisher>(session, new Book{ "The Catcher in the Rye" }, "Salinger");
		addEntry<Book, Publisher>(session, new Book{ "Pride and Prejudice" }, "Auste");

		addEntry<Shop>(session, new Shop{ "Altufevo" });
		addEntry<Shop>(session, new Shop{ "Brateevo" });
		addEntry<Shop>(session, new Shop{ "Chertanovo" });
		addEntry<Shop>(session, new Shop{ "Khimki" });

		addEntry<Stock, Book, Shop>(session, new Stock{ 11 }, "1984", "Altufevo");
		addEntry<Stock, Book, Shop>(session, new Stock{ 22 }, "To Kill a Mockingbird", "Altufevo");
		addEntry<Stock, Book, Shop>(session, new Stock{ 33 }, "To Kill a Mockingbird", "Chertanovo");
		addEntry<Stock, Book, Shop>(session, new Stock{ 44 }, "To Kill a Mockingbird", "Brateevo");
		addEntry<Stock, Book, Shop>(session, new Stock{ 55 }, "Animal Farm", "Khimki");
		addEntry<Stock, Book, Shop>(session, new Stock{ 66 }, "Animal Farm", "Chertanovo");
		addEntry<Stock, Book, Shop>(session, new Stock{ 77 }, "The Great Gatsby", "Altufevo");
		addEntry<Stock, Book, Shop>(session, new Stock{ 88 }, "The Great Gatsby", "Chertanovo");
		addEntry<Stock, Book, Shop>(session, new Stock{ 99 }, "The Catcher in the Rye", "Altufevo");
		addEntry<Stock, Book, Shop>(session, new Stock{ 111 }, "The Catcher in the Rye", "Khimki");
		addEntry<Stock, Book, Shop>(session, new Stock{ 122 }, "The Catcher in the Rye", "Brateevo");
		addEntry<Stock, Book, Shop>(session, new Stock{ 133 }, "Pride and Prejudice", "Brateevo");

		addEntry<Sale, Stock>(session, new Sale{ 35, "01.01.2025", 1 }, 0);
		addEntry<Sale, Stock>(session, new Sale{ 47, "01.01.2025", 2 }, 3);
		addEntry<Sale, Stock>(session, new Sale{ 10, "01.01.2025", 1 }, 5);
		addEntry<Sale, Stock>(session, new Sale{ 21, "01.01.2025", 2 }, 7);
		addEntry<Sale, Stock>(session, new Sale{ 27, "01.01.2025", 1 }, 8);
		addEntry<Sale, Stock>(session, new Sale{ 54, "01.01.2025", 3 }, 9);

		std::string publisher_name{};
		std::cout << "Enter publisher name: ";
		std::cin >> publisher_name;

		auto shops = getShops<Publisher>(session, publisher_name);

		int idx{};
		std::cout << std::endl << "Shop list:" << std::endl;
		for (const auto& shop : shops)
		{
			std::cout << idx << ": " << shop << std::endl;
			idx++;
		}		
	}
	catch (const dbo::Exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}

	std::cout << std::endl;
	system("pause");
	return EXIT_SUCCESS;
}