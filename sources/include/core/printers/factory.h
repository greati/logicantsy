#ifndef __PRINTER_FACTORY__
#define __PRINTER_FACTORY__

#include "core/printers/std.h"
#include "core/printers/latex.h"

namespace ltsy {

    /* Interface for printer objects.
     * */
    class PrinterFactory {

        public:

            std::shared_ptr<Printer> make_printer(Printer::PrinterType printer_type, const std::map<std::string, std::string>& translation) {
                std::shared_ptr<Printer> printer;
                switch(printer_type) {
                    case Printer::PrinterType::PLAIN:
                        printer = std::make_shared<StdPrinter>(translation);
                        break;
                    case Printer::PrinterType::LATEX:
                        printer = std::make_shared<LaTeXPrinter>(translation);
                        break;
                }
                return printer;
            };

    };
}

#endif
