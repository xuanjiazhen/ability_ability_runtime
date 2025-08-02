import cryptoHmacOptParaJsTestUnit from './HmacOptParameter.test'
import cryptoPbkdf2JsTestUnit from './Pbkdf2.test'
export default function testsuite() {
  cryptoHmacOptParaJsTestUnit();
  cryptoPbkdf2JsTestUnit();
}