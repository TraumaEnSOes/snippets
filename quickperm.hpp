#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

template< typename T, typename CB >
bool quickPerm( T *a, size_t N, CB &&cb ) {
    if( !N ) {
        throw std::length_error( "N must be > 0" );
    }

    if( !cb( a, N ) ) {
        return false;
    }

    std::vector< size_t > p( N + 1U );
    for( size_t idx = 1U; idx <= N; ++idx ) {
        p[idx] = idx;
    }

    size_t i = 1U;
    while( i < N ) {
        p[i]--;
        size_t j = i % 2 * p[1];
        std::swap( a[j], a[i] );

        i = 1U;
        while( !p[i] ) {
            p[i] = i;
            i++;
        }

        if( !cb( a, N ) ) {
            return false;
        }
    }

    return true;
}
