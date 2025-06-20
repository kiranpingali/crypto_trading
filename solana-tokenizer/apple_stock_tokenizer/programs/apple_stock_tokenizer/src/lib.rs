use anchor_lang::prelude::*;
use anchor_spl::token::{self, Mint, Token, TokenAccount, MintTo, InitializeMint};

declare_id!("HB2aE2CzJ5SMmifMmx6gxae81A92UTc2XMDfzxuFLU1M");

#[program]
pub mod apple_stock_tokenizer {
    use super::*;

    pub fn initialize_mint(
        ctx: Context<InitializeMintCtx>,
        decimals: u8,
        amount: u64,
    ) -> Result<()> {
        // Initialize the mint
        token::initialize_mint(
            CpiContext::new(
                ctx.accounts.token_program.to_account_info(),
                InitializeMint {
                    mint: ctx.accounts.mint.to_account_info(),
                    rent: ctx.accounts.rent.to_account_info(),
                },
            ),
            decimals,
            ctx.accounts.authority.key,
            Some(ctx.accounts.authority.key),
        )?;
        // Mint the initial supply to the destination account
        token::mint_to(
            CpiContext::new(
                ctx.accounts.token_program.to_account_info(),
                MintTo {
                    mint: ctx.accounts.mint.to_account_info(),
                    to: ctx.accounts.destination.to_account_info(),
                    authority: ctx.accounts.authority.to_account_info(),
                },
            ),
            amount,
        )?;
        Ok(())
    }
}

#[derive(Accounts)]
pub struct InitializeMintCtx<'info> {
    #[account(init, payer = authority, space = 82, mint::decimals = 0, mint::authority = authority)]
    pub mint: Account<'info, Mint>,
    #[account(mut)]
    pub destination: Account<'info, TokenAccount>,
    #[account(mut)]
    pub authority: Signer<'info>,
    pub rent: Sysvar<'info, Rent>,
    pub token_program: Program<'info, Token>,
    pub system_program: Program<'info, System>,
}
