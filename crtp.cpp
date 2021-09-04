#include <iostream>

struct C1 {
    void *data;
    int a;
};

struct C2 {
    void *data;
    int a;
    int b;
};

struct CppBase {
    unsigned locks = 1U;
};

struct Cpp1 : public CppBase {
    int data1;
};

struct Cpp2 : public Cpp1 {
    int data2;
};

struct Store1 {
    C1 c;
    Cpp1 cpp;
};

struct Store2 {
    C2 c;
    Cpp2 cpp;
};

struct Wrapper {
    void *m_c = nullptr;

    void *data( ) { return *reinterpret_cast< void ** >( m_c ); }
    void setData( void *ptr ) { *reinterpret_cast< void ** >( m_c ) = ptr; }
};

template< typename DERIVED > struct Crtp : public Wrapper {
    ~Crtp( ) {
        unlock( );
    }

    void unlock( ) {
        auto &locks = reinterpret_cast< CppBase * >( data( ) )->locks;
        std::cout << "unlock( ): locks == " << locks << '\n';

        if( locks == 1U ) {
            static_cast< DERIVED * >( this )->destroy( );
        } else {
            --locks;
        }
    }
};

template< typename DERIVED > struct Level1 : public Crtp< DERIVED > {
    void destroy( ) {
        std::cout << "destroy( ) on Level1\n";

        static_cast< DERIVED * >( this )->destroy( );
    }
};

struct Level2 : public Level1< Level2 > {
    Level2( ) {
        auto store = new Store2;
        m_c = store;
        setData( &( store->cpp ) );
    }

    void destroy( ) {
        std::cout << "destroy( ) on Level2\n";

        delete reinterpret_cast< Store2 * >( m_c );
    }
};

int main( ) {
    Level2 l2;

    return 0;
}
