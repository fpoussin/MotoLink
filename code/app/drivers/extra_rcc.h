#ifndef _EXTRA_RCC_
#define _EXTRA_RCC_

/**
 * @name    DAC peripheral specific RCC operations
 * @{
 */
/**
 * @brief   Enables the DAC peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableDAC(lp) rccEnableAPB1(RCC_APB1ENR_DACEN, lp)

/**
 * @brief   Disables the DAC peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableDAC(lp) rccDisableAPB1(RCC_APB1ENR_DACEN, lp)

/**
 * @brief   Resets the DAC peripheral.
 *
 * @api
 */
#define rccResetDAC() rccResetAPB1(RCC_APB1RSTR_DACRST)
/** @} */

#endif
