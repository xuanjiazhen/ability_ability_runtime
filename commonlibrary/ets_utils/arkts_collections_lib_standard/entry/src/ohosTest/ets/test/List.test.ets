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
}