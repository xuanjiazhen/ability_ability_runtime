/*
 * Type declarations for libpoc.so NAPI module.
 *
 * directExecuteIntent: bypasses AbilityManagerClient / AbilityManagerProxy entirely,
 * constructs MessageParcel by hand, and sends EXECUTE_INTENT (code 72) directly to
 * AbilityManagerService via raw binder.
 *
 * @param userId   value written into InsightIntentExecuteParam.userId_
 * @param key      value written as EXECUTE_INTENT key param (any uint64)
 * @returns        the int32 the service wrote back in reply.
 *                 < 0 means local-side failure (parcel/samgr/SendRequest err).
 *                 >= 0 is a real AbilityManagerService error code; non-zero usually
 *                 means service-side check rejected us (CHECK_PERMISSION_FAILED etc).
 */
export const directExecuteIntent: (userId: number, key?: number) => number;
