#pragma once
#include "instruction.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <print>
#include <span>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "jnvm"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#define LOGD(...) std::println(__VA_ARGS__)
#define LOGE(...) std::println(stderr, __VA_ARGS__)
#endif

namespace jnvm::machine
{
    using namespace jnvm::inst;

    constexpr std::size_t REG_CNT { 256 };
    constexpr std::size_t MAX_DEPTH { 1024 };

    class RuntimeError final : public std::runtime_error
    {
    public:
        explicit RuntimeError( const char* msg )
            : std::runtime_error { msg }
        {}
    };

    struct Frame
    {
        std::size_t ret_addr;
        uint8_t fp;
        uint8_t param_cnt;
        uint8_t res_reg;
        std::array<uint32_t, REG_CNT> regs { };

        Frame( std::size_t ret, uint8_t fp_, uint8_t params, uint8_t res, const std::array<uint32_t, REG_CNT>& r )
            : ret_addr( ret ), fp( fp_ ), param_cnt( params ), res_reg( res ), regs( r )
        {}
    };

    using VMNative = std::function<void(
        std::array<uint32_t, REG_CNT>&,
        uint32_t,
        uint8_t,
        const std::vector<std::string>&
    )>;

    class Machine
    {
    public:
        explicit Machine( bool dbg = false ) : m_dbg( dbg )
        {
            init_natives( );
        }

        void load( const std::vector<uint32_t>& bc )
        {
            m_bc = bc;
            reset( );
        }

        void load_strs( const std::vector<std::string>& strs )
        {
            m_strs = strs;
        }

        void reg_native( VMNativeID id, VMNative fn )
        {
            m_natives[ id ] = std::move( fn );
        }

        [[nodiscard]]
        uint32_t exec( )
        {
            if ( m_bc.empty( ) ) throw RuntimeError( "no bytecode" );

            while ( m_pc < m_bc.size( ) )
            {
                exec_one( );
                if ( m_halt )
                {
                    if ( m_dbg ) LOGD( "regs: final state" );
                    return m_regs[ 0 ];
                }
            }

            throw RuntimeError( "no hlt instruction" );
        }

    private:
        std::array<uint32_t, REG_CNT> m_regs { };
        std::vector<uint32_t> m_bc;
        std::vector<std::string> m_strs;
        std::size_t m_pc { 0 };
        std::size_t m_fp { 0 };
        bool m_halt { false };
        bool m_dbg { false };

        std::vector<Frame> m_stk;
        std::unordered_map<VMNativeID, VMNative> m_natives;

        std::chrono::steady_clock::time_point m_prof_start;
        std::size_t m_inst_cnt { 0 };

        void reset( )
        {
            m_regs.fill( 0 );
            m_pc = 0;
            m_fp = 0;
            m_halt = false;
            m_inst_cnt = 0;
            m_stk.clear( );
            m_stk.reserve( MAX_DEPTH );
        }

        inline void mov( const Instruction i )
        {
            m_regs[ i.op1( ) ] = i.op2( );
        }

        inline void copy( const Instruction i )
        {
            m_regs[ i.op1( ) ] = m_regs[ i.op2( ) ];
        }

        inline void loads( const Instruction i )
        {
            const auto idx { i.op2( ) };
            if ( idx >= m_strs.size( ) ) throw RuntimeError( "str idx oob" );
            m_regs[ i.op1( ) ] = make_idx_for_string( idx );
        }

        inline void add( const Instruction i )
        {
            m_regs[ i.op3( ) ] = m_regs[ i.op1( ) ] + m_regs[ i.op2( ) ];
        }

        inline void sub( const Instruction i )
        {
            m_regs[ i.op3( ) ] = m_regs[ i.op1( ) ] - m_regs[ i.op2( ) ];
        }

        inline void mul( const Instruction i )
        {
            m_regs[ i.op3( ) ] = m_regs[ i.op1( ) ] * m_regs[ i.op2( ) ];
        }

        inline void div( const Instruction i )
        {
            const auto rhs { m_regs[ i.op2( ) ] };
            if ( rhs == 0 ) throw RuntimeError( "div by zero" );
            m_regs[ i.op3( ) ] = m_regs[ i.op1( ) ] / rhs;
        }

        inline void inc( const Instruction i )
        {
            ++m_regs[ i.op1( ) ];
        }

        inline void jmp( const Instruction i )
        {
            m_pc = i.op1( );
        }

        inline void jnz( const Instruction i )
        {
            m_pc = m_regs[ i.op1( ) ] != 0 ? i.op2( ) : m_pc + 1;
        }

        inline void jz( const Instruction i )
        {
            m_pc = m_regs[ i.op1( ) ] == 0 ? i.op2( ) : m_pc + 1;
        }

        inline void eq( const Instruction i )
        {
            m_regs[ i.op3( ) ] = m_regs[ i.op1( ) ] == m_regs[ i.op2( ) ];
        }

        inline void neq( const Instruction i )
        {
            m_regs[ i.op3( ) ] = m_regs[ i.op1( ) ] != m_regs[ i.op2( ) ];
        }

        inline void lt( const Instruction i )
        {
            m_regs[ i.op3( ) ] = m_regs[ i.op1( ) ] < m_regs[ i.op2( ) ];
        }

        inline void gt( const Instruction i )
        {
            m_regs[ i.op3( ) ] = m_regs[ i.op1( ) ] > m_regs[ i.op2( ) ];
        }

        inline void lte( const Instruction i )
        {
            m_regs[ i.op3( ) ] = m_regs[ i.op1( ) ] <= m_regs[ i.op2( ) ];
        }

        inline void gte( const Instruction i )
        {
            m_regs[ i.op3( ) ] = m_regs[ i.op1( ) ] >= m_regs[ i.op2( ) ];
        }

        void exec_native( uint8_t addr, uint8_t base, uint8_t argc )
        {
            const auto fn { static_cast<VMNativeID>( addr ) };
            auto it { m_natives.find( fn ) };
            if ( it == m_natives.end( ) ) throw RuntimeError( "unknown native" );
            it->second( m_regs, base, argc, m_strs );
        }

        void exec_usr( uint8_t addr, uint8_t base, uint8_t argc )
        {
            if ( m_stk.size( ) >= MAX_DEPTH ) throw RuntimeError( "stk overflow" );
            m_stk.emplace_back( m_pc + 1, m_fp, argc, base, m_regs );
            m_fp = base;
            m_pc = addr;
        }

        void call( const Instruction i )
        {
            const auto addr { i.op1( ) };
            const auto base { i.op2( ) };
            const auto argc { i.op3( ) };

            if ( is_vm_native( addr ) )
            {
                exec_native( addr, base, argc );
                ++m_pc;
            }
            else
            {
                exec_usr( addr, base, argc );
            }
        }

        void ret( const Instruction i )
        {
            if ( m_stk.empty( ) )
            {
                m_halt = true;
                return;
            }

            const auto val { m_regs[ 0 ] };
            const auto& f { m_stk.back( ) };
            
            m_regs = f.regs;
            m_pc = f.ret_addr;
            m_fp = f.fp;
            m_regs[ f.res_reg ] = val;
            
            m_stk.pop_back( );
        }

        inline void prf( const Instruction i )
        {
            m_prof_start = std::chrono::steady_clock::now( );
        }

        void prfe( const Instruction i )
        {
            const auto end { std::chrono::steady_clock::now( ) };
            const auto dur { std::chrono::duration_cast<std::chrono::milliseconds>( end - m_prof_start ) };
            LOGD( "exec: %llums, insts: %zu", dur.count( ), m_inst_cnt );
        }

        inline void hlt( const Instruction i )
        {
            m_halt = true;
        }

        void exec_one( )
        {
            if ( m_pc >= m_bc.size( ) ) throw RuntimeError( "pc oob" );

            const Instruction i { m_bc[ m_pc ] };
            ++m_inst_cnt;

            switch ( i.opcode( ) )
            {
                case Opcode::MOV:   mov( i );   ++m_pc; break;
                case Opcode::COPY:  copy( i );  ++m_pc; break;
                case Opcode::LOADS: loads( i ); ++m_pc; break;
                case Opcode::ADD:   add( i );   ++m_pc; break;
                case Opcode::SUB:   sub( i );   ++m_pc; break;
                case Opcode::MUL:   mul( i );   ++m_pc; break;
                case Opcode::DIV:   div( i );   ++m_pc; break;
                case Opcode::INC:   inc( i );   ++m_pc; break;
                case Opcode::JMP:   jmp( i );   break;
                case Opcode::JNZ:   jnz( i );   break;
                case Opcode::JZ:    jz( i );    break;
                case Opcode::EQ:    eq( i );    ++m_pc; break;
                case Opcode::NEQ:   neq( i );   ++m_pc; break;
                case Opcode::LT:    lt( i );    ++m_pc; break;
                case Opcode::GT:    gt( i );    ++m_pc; break;
                case Opcode::LTE:   lte( i );   ++m_pc; break;
                case Opcode::GTE:   gte( i );   ++m_pc; break;
                case Opcode::CALL:  call( i );  break;
                case Opcode::RET:   ret( i );   break;
                case Opcode::PRF:   prf( i );   ++m_pc; break;
                case Opcode::PRFE:  prfe( i );  ++m_pc; break;
                case Opcode::HLT:   hlt( i );   return;
                default: throw RuntimeError( "unknown opcode" );
            }
        }

        void init_natives( )
        {
            reg_native(
                VMNativeID::PRINT,
                []( auto& regs, uint32_t base, uint8_t argc, const auto& strs )
                {
                    for ( uint8_t i { 0 }; i < argc; ++i )
                    {
                        const auto val { regs[ base + i ] };
                        if ( is_string_value( val ) )
                        {
                            const auto idx { get_string_idx( val ) };
                            if ( idx < strs.size( ) )
                            {
#ifdef __ANDROID__
                                LOGD( "%s ", strs[ idx ].c_str( ) );
#else
                                std::print( "{} ", strs[ idx ] );
#endif
                            }
                        }
                        else
                        {
#ifdef __ANDROID__
                            LOGD( "%u ", val );
#else
                            std::print( "{} ", val );
#endif
                        }
                    }
#ifndef __ANDROID__
                    std::cout << std::endl;
#endif
                }
            );
        }
    };
}
