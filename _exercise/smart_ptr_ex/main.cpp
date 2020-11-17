#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace std;

class Gadget
{
public:
    // konstruktor
    Gadget(int id = 0)
        : id_ {id}
    {
        std::cout << "Konstruktor Gadget(" << id_ << ")\n";
    }

    // destruktor
    ~Gadget()
    {
        std::cout << "Destruktor ~Gadget(" << id_ << ")\n";
    }

    int id() const
    {
        return id_;
    }

    void set_id(int id)
    {
        id_ = id;
    }

    void use()
    {
        std::cout << "Using a gadget with id: " << id() << '\n';
    }

    void unsafe()
    {
        std::cout << "Using a gadget with id: " << id() << " - Ups... It crashed..." << std::endl;
        throw std::runtime_error("ERROR");
    }

private:
    int id_;
};

namespace LegacyCode
{
    Gadget* create_many_gadgets(unsigned int size)
    {
        Gadget* many_gadgets = new Gadget[size];

        for (unsigned int i = 0; i < size; ++i)
            many_gadgets[i].set_id(i);

        return many_gadgets;
    }
}

void reset_value(Gadget& g, int n)
{
    // some logic

    g.set_id(n);
    cout << "New id for Gadget: " << g.id() << endl;
}

//////////////////////////////////////////////
// TODO - modernize the code below

std::unique_ptr<Gadget> create_gadget(int arg)
{
    return std::make_unique<Gadget>(arg);
}

class Player
{
    std::unique_ptr<Gadget> gadget_;
    std::ostream* logger_;

public:
    Player(std::unique_ptr<Gadget> g, std::ostream* logger = nullptr)
        : gadget_ {std::move(g)}
        , logger_ {logger}
    {
        if (!gadget_)
            throw std::invalid_argument("Gadget can not be null");
    }

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    Player(Player&& p) noexcept
        : gadget_ {std::move(p.gadget_)}
        , logger_ {std::move(p.logger_)}
    {
        p.logger_ = nullptr;
    }

    Player& operator=(Player&& p) noexcept
    {
        if (this != &p)
        {
            gadget_ = std::move(p.gadget_);
            logger_ = std::move(p.logger_);
            p.logger_ = nullptr;
        }

        return *this;
    }

    ~Player()
    {
        if (logger_)
            *logger_ << "Destroing a gadget: " << gadget_->id() << std::endl;
    }

    void play()
    {
        if (logger_)
            *logger_ << "Player is using a gadget: " << gadget_->id() << std::endl;

        gadget_->use();
    }
};

void unsafe1() // TODO: poprawa z wykorzystaniem smart_ptr
{
    std::unique_ptr<Gadget> ptr_gdgt = create_gadget(4);

    /* kod korzystajacy z ptrX */

    reset_value(*ptr_gdgt, 5);

    ptr_gdgt->unsafe();
}

void unsafe2() // TODO: poprawa z wykorzystaniem smart_ptr
{
    int size = 10;

    std::unique_ptr<Gadget[]> buffer{LegacyCode::create_many_gadgets(10)};
    /* kod korzystający z buffer */

    for (int i = 0; i < size; ++i)
        buffer[0].unsafe();
}

void unsafe3()
{
    vector<std::unique_ptr<Gadget>> my_gadgets;

    my_gadgets.emplace_back(create_gadget(87));
    my_gadgets.emplace_back(create_gadget(12));
    my_gadgets.emplace_back(std::make_unique<Gadget>(98));
    

    int value_generator = 0;
    for (auto& g : my_gadgets)
    {
        cout << "Gadget's old id: " << g->id() << endl;
        reset_value(*g, ++value_generator);
    }

    my_gadgets[0].reset();

    Player p(std::move(my_gadgets.back()));
    p.play();

    my_gadgets[1]->unsafe();
}

int main()
try
{
    try
    {
        //unsafe1();
        //unsafe2();
        unsafe3();
    }
    catch (const exception& e)
    {
        cout << "Exception caught: " << e.what() << endl;
    }
}
catch (const exception& e)
{
    cout << "Exception caught: " << e.what() << endl;
}