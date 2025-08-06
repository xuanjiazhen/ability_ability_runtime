/*
* Copyright (C) 2025 HiHope Open Source Organization.
* Licensed under the Apache License, Version 2.0 (the 'License');
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an 'AS IS' BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
import abilityTest from './Ability.test';
import AsyncLocktest from './AsyncLocktest.test';
import BitVectortest from './BitVector.test';
import Uint8ClampedArraytest from './ClampedArrayTest.test';
import collectionsFifthtest from './CollectionsFifithTest.test';
import CollectionsFloattest from './CollectionsFloattest.test';
import collectionsForthtest from './CollectionsForthTest.test';
import collectionsInttest from './CollectionsInttest.test';
import collectionsSecondtest from './CollectionsSecondtest.test';
import collectionsSixtest from './CollectionsSixTest.test';
import collectionstest from './Collectionstest.test';
import collectionsThirdtest from './CollectionsThirdtest.test';
import sendableLruCachetest from './SendableLruCachetest.test';
import collectionsAddTest from './CollectionsAdd.test';
import collectionsConcurrentModifyTest from './CollectionsConcurrentModify.test';

export default function testsuite() {
  abilityTest();
  collectionstest();
  collectionsSecondtest();
  collectionsThirdtest();
  collectionsForthtest();
  collectionsFifthtest();
  collectionsSixtest();
  sendableLruCachetest();
  CollectionsFloattest();
  collectionsInttest();
  Uint8ClampedArraytest();
  BitVectortest();
  AsyncLocktest();
  collectionsAddTest();
  collectionsConcurrentModifyTest()
}