import argparse

from jinja2 import Template


def read_cli():
    parser = argparse.ArgumentParser(description='Generate lala source code.')
    parser.add_argument('grammar-file', help='The file with lala grammar')
    return parser.parse_args(sys.argv)

def read_tokens_from_grammar_file(grammar_file_name):
    pass
    return tokens

def main():
    args = read_cli()
    tokens: Map[str: str] = read_tokens_from_grammar_file(args.grammar_file)

    generate_token_src(tokens)
    generate_lexer_src(tokens)


if __name__ == '__main__':
    main()

