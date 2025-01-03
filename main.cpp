#include <iostream>
#include <string>
#include <vector>

#include "data/document_store.hpp"
#include "text_processing/duplicate_finder.hpp"
#include "sql/sql_handler.hpp"

void print_usage() {
    std::cerr << "Usage: duplicate_finder [-v|--verbose] <database_path> <output_json_path> <domain> <threshold>" << std::endl;
    std::cerr << "Usage: duplicate_finder <database_path> <output_json_path> <domain> <threshold>" << std::endl;
    std::cerr << "  <database_path>: Path to SQLite database" << std::endl;
    std::cerr << "  <output_json_path>: Path to save JSON output" << std::endl;
    std::cerr << "  <domain>: Domain to filter documents" << std::endl;
    std::cerr << "  <threshold>: Minimum duplicate substring length" << std::endl;
}

int main(int argc, char* argv[]) {
    // Check for correct number of arguments
    if (argc < 5 || argc > 6) {
        print_usage();
        return 1;
    }

    try {
        // Parse arguments
        bool verbose = false;
        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "-v" || std::string(argv[i]) == "--verbose") {
                verbose = true;
                break;
            }
        }
        std::string db_path = argv[argc-4];
        std::string output_path = argv[argc-3];
        std::string domain = argv[argc-2];
        size_t threshold = std::stoull(argv[argc-1]);


        if (verbose) std::cout << "Creating SQLite Handler..." << std::endl;
        // Create SQLite Handler
        text_processing::SQLiteHandler sql_handler(db_path);

        if (verbose) std::cout << "Validating..." << std::endl;
        // Validate table and columns exist
        auto [valid, error] = sql_handler.validateTableAndColumns(
            "data_table",
            {"domains", "doc_content"}
        );

        if (!valid) {
            std::cerr << "Database validation failed: " << error << std::endl;
            return 1;
        }

        if (verbose) std::cout << "Creating DocumentStore..." << std::endl;
        // Create document store filtered by domain
        auto store = sql_handler.createDocumentStore(
            "data_table",   // table name
            "domains",      // filter column
            "doc_content",     // content column
            domain         // filter value
        );

        if (verbose) std::cout << "Finding duplicates..." << std::endl;
        // Find duplicates
        text_processing::DuplicateFinder finder;
        auto matches = finder.find_duplicates(store, threshold);

        if (verbose) std::cout << "Saving Results..." << std::endl;
        // Save matches to JSON
        text_processing::DuplicateFinder::save_matches_to_json(matches, output_path);

        std::cout << "Found " << matches.size() << " duplicate matches. Saved to " << output_path << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}