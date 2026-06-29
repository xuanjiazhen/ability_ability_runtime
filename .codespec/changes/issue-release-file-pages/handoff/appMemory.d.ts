/**
 * @file
 * @kit AbilityKit
 *
 * Handoff artifact for interface/sdk-js repo (O-4). Drop-in location:
 *   api/@ohos.app.ability.appMemory.d.ts  (ets)
 *   api/@ohos.app.ability.appMemory.d.ts  (js)
 *
 * The native bindings live in ability_runtime:
 *   frameworks/ets/ani/app_memory/src/ets_app_memory.cpp   (ANI)
 *   frameworks/js/napi/app_memory/js_app_memory.cpp        (NAPI)
 *
 * Native function names bound via Namespace_BindNativeFunctions:
 *   nativeReleaseFilePages(callback, fileNames)
 *   nativeReleaseRawFilePages(callback, moduleName)
 *
 * @since value to be assigned by the SDK team (use the next API version banner).
 */
import { AsyncCallback } from './@ohos.base';

/**
 * Provides the ability to release file-backed pages of the calling application's
 * own bundle on demand, to reduce memory footprint.
 *
 * @namespace appMemory
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @systemapi
 * @since {{TBD}}
 */
declare namespace appMemory {
    /**
     * Release file pages of the specified hap/hsp/so files belonging to the
     * calling bundle. Pages of read-only, non-writable mappings are released
     * via MADV_DONTNEED and can be reloaded from the file on next access.
     *
     * Validation is atomic: if any entry is illegal (bad suffix, contains a
     * path separator, or is not present in the caller's own bundle), nothing
     * is released.
     *
     * @param { Array<string> } fileNames - plain file names (e.g. 'libfoo.so',
     *     'entry.hap'). Path separators and '..' are not allowed.
     * @returns { Promise<void> } The promise returned by the function.
     * @throws { BusinessError } 202 - Not system application.
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     *     1. Mandatory parameters are left unspecified; 2. Incorrect parameter types;
     *     3. Parameter verification failed (empty array, path separator, unsupported
     *     suffix, or file not in caller bundle).
     * @throws { BusinessError } 16000050 - Internal error.
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @since {{TBD}}
     */
    function releaseFilePages(fileNames: Array<string>): Promise<void>;

    /**
     * Release file pages of the top-level rawfile entries of the specified
     * module of the calling bundle. Subdirectories inside rawfile are NOT
     * released.
     *
     * @param { string } moduleName - module name (the `name` field in module.json5).
     * @returns { Promise<void> } The promise returned by the function.
     * @throws { BusinessError } 202 - Not system application.
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     *     1. Mandatory parameters are left unspecified; 2. Incorrect parameter types;
     *     3. Parameter verification failed (empty module name, illegal characters,
     *     or module not found in caller bundle).
     * @throws { BusinessError } 16000050 - Internal error.
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @since {{TBD}}
     */
    function releaseRawFilePages(moduleName: string): Promise<void>;
}

export default appMemory;
