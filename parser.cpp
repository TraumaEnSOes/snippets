#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <unordered_set>

struct Item {
    Item( const char *name, size_t size ) :
        itemName( name ),
        itemSize( size )
    {
    }

    const Item *next( ) const noexcept {
        if( itemSize == 0 ) {
            return nullptr;
        }

        const char *ptr = reinterpret_cast< const char * >( this );
        ptr += itemSize;
        return reinterpret_cast< const Item * >( ptr );
    }

    const char *itemName;
    const size_t itemSize;

    virtual bool validate( ) const = 0;
};

struct Begin : public Item {
    Begin( ) : Item( "BEGIN", sizeof( *this ) ) { }

    bool validate( ) const override;
};
bool Begin::validate( ) const {
    throw std::runtime_error( "Internal error: Try validate the BEGIN" );
    return false;
}

struct END : public Item {
    END( ) : Item( "END", 0 ) { }

    bool validate( ) const override;
};
bool END::validate( ) const {
    throw std::runtime_error( "Internal error: Try validate the END" );
    return false;
}

template< typename T > struct ItemBase : public Item {
    ItemBase( const char *name ) :
        Item( name, sizeof( T ) )
    {
    }
};

template< typename T > struct Atom : public ItemBase< Atom< T > > {
    Atom( const char *name, std::optional< T > defValue = std::optional< T >( ) ) :
        ItemBase< Atom< T > >( name ),
        defaultValue( std::move( defValue ) )
    {
    }
    Atom( const char *name, T defValue ) :
        Atom( name, std::optional< T >( std::in_place, std::move( defValue ) ) )
    {
    }

    const std::optional< T > defaultValue;

private:
    bool validate( ) const override { return true; }
};

template< typename T > struct GenericEnum : public ItemBase< GenericEnum< T > > {
    GenericEnum( const char *name, std::initializer_list< T > values, std::optional< T > defValue = std::optional< T >( ) ) :
        ItemBase< GenericEnum< T > >( name ),
        validValues( values ),
        defaultValue( std::move( defValue ) )
    {
    }
    GenericEnum( const char *name, std::initializer_list< T > values, T defValue ) :
        GenericEnum( name, values, std::optional< T >( std::in_place, std::move( defValue ) ) )
    {
    }

    const std::unordered_set< T > validValues;
    std::optional< T > defaultValue;

private:
    bool validate( ) const override { return true; }
};

using Enum = GenericEnum< int >;

struct DocumentBase : public Item {
    DocumentBase( const char *name, size_t size ) :
        Item( name, size )
    {
    }

    bool validate( ) const override {
        bool result;

        const Item *itemPtr = begin.next( );

        while( itemPtr->itemSize != 0 ) {
            std::cout << "Validando elemento " << itemPtr->itemName << '\n';

            result = itemPtr->validate( );

            if( !result ) { break; }

            itemPtr = itemPtr->next( );
        }

        return result;
    }

private:
    Begin begin;
};

#define BEGIN_DOCUMENT( CLASS, NAME ) struct CLASS : public DocumentBase { CLASS( ) : DocumentBase( NAME, sizeof( *this ) ) { }

#define END_DOCUMENT END end; };

BEGIN_DOCUMENT( Document, "TestDocument" )
    Atom< int > item1{ "Atom::item1", -20 };
    Atom< double > item2{ "Atom::item2", 10.0 };
    Enum item3{ "Enum::item3", std::initializer_list< int >{ 1, 2, 3, 4, 5 }, 4 };
END_DOCUMENT

int main( ) {
    Document doc;

    std::cout << ( doc.validate( ) ? "Validacion correcta !\n" : "Validacion erronea !\n" );

    return 0;
}

